# $Id$
#
#  Copyright (c) 2007-2013, Novartis Institutes for BioMedical Research Inc.
#  All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: 
#
#     * Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following 
#       disclaimer in the documentation and/or other materials provided 
#       with the distribution.
#     * Neither the name of Novartis Institutes for BioMedical Research Inc. 
#       nor the names of its contributors may be used to endorse or promote 
#       products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  Created by Greg Landrum, July 2007
#
from __future__ import print_function

usage = """
 SearchDb [optional arguments] <sdfilename>

     The sd filename argument can be either an SD file or an MDL mol 
     file.
     

  NOTES:

    - The property names may have been altered on loading the
      database.  Any non-alphanumeric character in a property name
      will be replaced with '_'. e.g."Gold.Goldscore.Constraint.Score" becomes
      "Gold_Goldscore_Constraint_Score".

    - Property names are not case sensitive in the database.

"""

from rdkit import RDConfig
from rdkit.Dbase.DbConnection import DbConnect

from rdkit.RDLogger import logger
logger = logger()
import zlib
from rdkit import Chem

from rdkit.Chem.MolDb.FingerprintUtils import supportedSimilarityMethods, BuildSigFactory, DepickleFP, LayeredOptions
from rdkit.Chem.MolDb import FingerprintUtils

from rdkit import DataStructs


def _molFromPkl(pkl):
  if isinstance(pkl, (bytes, str)):
    mol = Chem.Mol(pkl)
  else:
    mol = Chem.Mol(str(pkl))
  return mol


def GetNeighborLists(probes, topN, pool, simMetric=DataStructs.DiceSimilarity, simThresh=-1.,
                     silent=False, **kwargs):
  probeFps = [x[1] for x in probes]
  validProbes = [x for x in range(len(probeFps)) if probeFps[x] is not None]
  validFps = [probeFps[x] for x in validProbes]
  from rdkit.DataStructs.TopNContainer import TopNContainer
  if simThresh <= 0:
    nbrLists = [TopNContainer(topN) for x in range(len(probeFps))]
  else:
    nbrLists = [TopNContainer(-1) for x in range(len(probeFps))]

  nDone = 0
  for nm, fp in pool:
    nDone += 1
    if not silent and not nDone % 1000:
      logger.info('  searched %d rows' % nDone)
    if (simMetric == DataStructs.DiceSimilarity):
      scores = DataStructs.BulkDiceSimilarity(fp, validFps)
      for i, score in enumerate(scores):
        if score > simThresh:
          nbrLists[validProbes[i]].Insert(score, nm)
    elif (simMetric == DataStructs.TanimotoSimilarity):
      scores = DataStructs.BulkTanimotoSimilarity(fp, validFps)
      for i, score in enumerate(scores):
        if score > simThresh:
          nbrLists[validProbes[i]].Insert(score, nm)
    elif (simMetric == DataStructs.TverskySimilarity):
      av = float(kwargs.get('tverskyA', 0.5))
      bv = float(kwargs.get('tverskyB', 0.5))
      scores = DataStructs.BulkTverskySimilarity(fp, validFps, av, bv)
      for i, score in enumerate(scores):
        if score > simThresh:
          nbrLists[validProbes[i]].Insert(score, nm)
    else:
      for i in range(len(probeFps)):
        pfp = probeFps[i]
        if pfp is not None:
          score = simMetric(probeFps[i], fp)
          if score > simThresh:
            nbrLists[validProbes[i]].Insert(score, nm)
  return nbrLists


def GetMolsFromSmilesFile(dataFilename, errFile, nameProp):
  dataFile = open(dataFilename, 'r')
  for idx, line in enumerate(dataFile):
    try:
      smi, nm = line.strip().split(' ')
    except ValueError:
      continue
    m = Chem.MolFromSmiles(smi)
    if not m:
      if errfile:
        print(idx, nm, smi, file=errfile)
      continue
    yield (nm, smi, m)


def GetMolsFromSDFile(dataFilename, errFile, nameProp):
  suppl = Chem.SDMolSupplier(dataFilename)

  for idx, m in enumerate(suppl):
    if not m:
      if errFile:
        if hasattr(suppl, 'GetItemText'):
          d = suppl.GetItemText(idx)
          errFile.write(d)
        else:
          logger.warning('full error file support not complete')
      continue
    smi = Chem.MolToSmiles(m, True)
    if m.HasProp(nameProp):
      nm = m.GetProp(nameProp)
      if not nm:
        logger.warning('molecule found with empty name property')
    else:
      nm = 'Mol_%d' % (idx + 1)
    yield nm, smi, m


def RunSearch(args, queryFilename):
  global sigFactory
  if args.similarityType == 'AtomPairs':
    fpBuilder = FingerprintUtils.BuildAtomPairFP
    simMetric = DataStructs.DiceSimilarity
    dbName = os.path.join(args.dbDir, args.pairDbName)
    fpTableName = args.pairTableName
    fpColName = args.pairColName
  elif args.similarityType == 'TopologicalTorsions':
    fpBuilder = FingerprintUtils.BuildTorsionsFP
    simMetric = DataStructs.DiceSimilarity
    dbName = os.path.join(args.dbDir, args.torsionsDbName)
    fpTableName = args.torsionsTableName
    fpColName = args.torsionsColName
  elif args.similarityType == 'RDK':
    fpBuilder = FingerprintUtils.BuildRDKitFP
    simMetric = DataStructs.FingerprintSimilarity
    dbName = os.path.join(args.dbDir, args.fpDbName)
    fpTableName = args.fpTableName
    if not args.fpColName:
      args.fpColName = 'rdkfp'
    fpColName = args.fpColName
  elif args.similarityType == 'Pharm2D':
    fpBuilder = FingerprintUtils.BuildPharm2DFP
    simMetric = DataStructs.DiceSimilarity
    dbName = os.path.join(args.dbDir, args.fpDbName)
    fpTableName = args.pharm2DTableName
    if not args.fpColName:
      args.fpColName = 'pharm2dfp'
    fpColName = args.fpColName
    FingerprintUtils.sigFactory = BuildSigFactory(args)
  elif args.similarityType == 'Gobbi2D':
    from rdkit.Chem.Pharm2D import Gobbi_Pharm2D
    fpBuilder = FingerprintUtils.BuildPharm2DFP
    simMetric = DataStructs.TanimotoSimilarity
    dbName = os.path.join(args.dbDir, args.fpDbName)
    fpTableName = args.gobbi2DTableName
    if not args.fpColName:
      args.fpColName = 'gobbi2dfp'
    fpColName = args.fpColName
    FingerprintUtils.sigFactory = Gobbi_Pharm2D.factory
  elif args.similarityType == 'Morgan':
    fpBuilder = FingerprintUtils.BuildMorganFP
    simMetric = DataStructs.DiceSimilarity
    dbName = os.path.join(args.dbDir, args.morganFpDbName)
    fpTableName = args.morganFpTableName
    fpColName = args.morganFpColName

  extraArgs = {}
  if args.similarityMetric == 'tanimoto':
    simMetric = DataStructs.TanimotoSimilarity
  elif args.similarityMetric == 'dice':
    simMetric = DataStructs.DiceSimilarity
  elif args.similarityMetric == 'tversky':
    simMetric = DataStructs.TverskySimilarity
    extraArgs['tverskyA'] = args.tverskyA
    extraArgs['tverskyB'] = args.tverskyB

  if args.smilesQuery:
    mol = Chem.MolFromSmiles(args.smilesQuery)
    if not mol:
      logger.error('could not build query molecule from smiles "%s"' % args.smilesQuery)
      sys.exit(-1)
    args.queryMol = mol
  elif args.smartsQuery:
    mol = Chem.MolFromSmarts(args.smartsQuery)
    if not mol:
      logger.error('could not build query molecule from smarts "%s"' % args.smartsQuery)
      sys.exit(-1)
    args.queryMol = mol

  if args.outF == '-':
    outF = sys.stdout
  elif args.outF == '':
    outF = None
  else:
    outF = open(args.outF, 'w+')

  molsOut = False
  if args.sdfOut:
    molsOut = True
    if args.sdfOut == '-':
      sdfOut = sys.stdout
    else:
      sdfOut = open(args.sdfOut, 'w+')
  else:
    sdfOut = None
  if args.smilesOut:
    molsOut = True
    if args.smilesOut == '-':
      smilesOut = sys.stdout
    else:
      smilesOut = open(args.smilesOut, 'w+')
  else:
    smilesOut = None

  if queryFilename:
    try:
      tmpF = open(queryFilename, 'r')
    except IOError:
      logger.error('could not open query file %s' % queryFilename)
      sys.exit(1)

    if args.molFormat == 'smiles':
      func = GetMolsFromSmilesFile
    elif args.molFormat == 'sdf':
      func = GetMolsFromSDFile

    if not args.silent:
      msg = 'Reading query molecules'
      if fpBuilder:
        msg += ' and generating fingerprints'
      logger.info(msg)
    probes = []
    i = 0
    nms = []
    for nm, smi, mol in func(queryFilename, None, args.nameProp):
      i += 1
      nms.append(nm)
      if not mol:
        logger.error('query molecule %d could not be built' % (i))
        probes.append((None, None))
        continue
      if fpBuilder:
        probes.append((mol, fpBuilder(mol)))
      else:
        probes.append((mol, None))
      if not args.silent and not i % 1000:
        logger.info("  done %d" % i)
  else:
    probes = None

  conn = None
  idName = args.molIdName
  ids = None
  names = None
  molDbName = os.path.join(args.dbDir, args.molDbName)
  molIdName = args.molIdName
  mConn = DbConnect(molDbName)
  cns = [(x.lower(), y) for x, y in mConn.GetColumnNamesAndTypes('molecules')]
  idCol, idTyp = cns[0]
  if args.propQuery or args.queryMol:
    conn = DbConnect(molDbName)
    curs = conn.GetCursor()
    if args.queryMol:
      if not args.silent:
        logger.info('Doing substructure query')
      if args.propQuery:
        where = 'where %s' % args.propQuery
      else:
        where = ''
      if not args.silent:
        curs.execute('select count(*) from molecules %(where)s' % locals())
        nToDo = curs.fetchone()[0]

      join = ''
      doSubstructFPs = False
      fpDbName = os.path.join(args.dbDir, args.fpDbName)
      if os.path.exists(fpDbName) and not args.negateQuery:
        curs.execute("attach database '%s' as fpdb" % (fpDbName))
        try:
          curs.execute('select * from fpdb.%s limit 1' % args.layeredTableName)
        except Exception:
          pass
        else:
          doSubstructFPs = True
          join = 'join fpdb.%s using (%s)' % (args.layeredTableName, idCol)
          query = LayeredOptions.GetQueryText(args.queryMol)
          if query:
            if not where:
              where = 'where'
            else:
              where += ' and'
            where += ' ' + query

      cmd = 'select %(idCol)s,molpkl from molecules %(join)s %(where)s' % locals()
      curs.execute(cmd)
      row = curs.fetchone()
      nDone = 0
      ids = []
      while row:
        id, molpkl = row
        if not args.zipMols:
          m = _molFromPkl(molpkl)
        else:
          m = Chem.Mol(zlib.decompress(molpkl))
        matched = m.HasSubstructMatch(args.queryMol)
        if args.negateQuery:
          matched = not matched
        if matched:
          ids.append(id)
        nDone += 1
        if not args.silent and not nDone % 500:
          if not doSubstructFPs:
            logger.info('  searched %d (of %d) molecules; %d hits so far' %
                        (nDone, nToDo, len(ids)))
          else:
            logger.info('  searched through %d molecules; %d hits so far' % (nDone, len(ids)))
        row = curs.fetchone()
      if not args.silent and doSubstructFPs and nToDo:
        nFiltered = nToDo - nDone
        logger.info('   Fingerprint screenout rate: %d of %d (%%%.2f)' %
                    (nFiltered, nToDo, 100. * nFiltered / nToDo))

    elif args.propQuery:
      if not args.silent:
        logger.info('Doing property query')
      propQuery = args.propQuery.split(';')[0]
      curs.execute('select %(idCol)s from molecules where %(propQuery)s' % locals())
      ids = [x[0] for x in curs.fetchall()]
    if not args.silent:
      logger.info('Found %d molecules matching the query' % (len(ids)))

  t1 = time.time()
  if probes:
    if not args.silent:
      logger.info('Finding Neighbors')
    conn = DbConnect(dbName)
    cns = conn.GetColumnNames(fpTableName)
    curs = conn.GetCursor()

    if ids:
      ids = [(x, ) for x in ids]
      curs.execute('create temporary table _tmpTbl (%(idCol)s %(idTyp)s)' % locals())
      curs.executemany('insert into _tmpTbl values (?)', ids)
      join = 'join  _tmpTbl using (%(idCol)s)' % locals()
    else:
      join = ''

    if cns[0].lower() != idCol.lower():
      # backwards compatibility to the days when mol tables had a guid and
      # the fps tables did not:
      curs.execute("attach database '%(molDbName)s' as mols" % locals())
      curs.execute("""
  select %(idCol)s,%(fpColName)s from %(fpTableName)s join
      (select %(idCol)s,%(molIdName)s from mols.molecules %(join)s)
    using (%(molIdName)s)
""" % (locals()))
    else:
      curs.execute('select %(idCol)s,%(fpColName)s from %(fpTableName)s %(join)s' % locals())

    def poolFromCurs(curs, similarityMethod):
      row = curs.fetchone()
      while row:
        id, pkl = row
        fp = DepickleFP(pkl, similarityMethod)
        yield (id, fp)
        row = curs.fetchone()

    topNLists = GetNeighborLists(probes, args.topN, poolFromCurs(curs, args.similarityType),
                                 simMetric=simMetric, simThresh=args.simThresh, **extraArgs)
    uniqIds = set()
    nbrLists = {}
    for i, nm in enumerate(nms):
      topNLists[i].reverse()
      scores = topNLists[i].GetPts()
      nbrNames = topNLists[i].GetExtras()
      nbrs = []
      for j, nbrGuid in enumerate(nbrNames):
        if nbrGuid is None:
          break
        else:
          uniqIds.add(nbrGuid)
          nbrs.append((nbrGuid, scores[j]))
      nbrLists[(i, nm)] = nbrs
    t2 = time.time()
    if not args.silent:
      logger.info('The search took %.1f seconds' % (t2 - t1))

# Please check %.1f

    if not args.silent:
      logger.info('Creating output')

    curs = mConn.GetCursor()
    ids = list(uniqIds)

    ids = [(x, ) for x in ids]
    curs.execute('create temporary table _tmpTbl (%(idCol)s %(idTyp)s)' % locals())
    curs.executemany('insert into _tmpTbl values (?)', ids)
    curs.execute('select %(idCol)s,%(molIdName)s from molecules join _tmpTbl using (%(idCol)s)' %
                 locals())
    nmDict = {}
    for guid, id in curs.fetchall():
      nmDict[guid] = str(id)

    ks = list(nbrLists.keys())
    ks.sort()
    if not args.transpose:
      for i, nm in ks:
        nbrs = nbrLists[(i, nm)]
        nbrTxt = args.outputDelim.join([nm] + ['%s%s%.3f' % (nmDict[id], args.outputDelim,
                                                                score) for id, score in nbrs])

        if outF:
          print(nbrTxt, file=outF)
    else:
      labels = ['%s%ssSimilarity' % (x[1], args.outputDelim) for x in ks]
      if outF:
        print(args.outputDelim.join(labels), file=outF)
      for i in range(args.topN):
        outL = []
        for idx, nm in ks:
          nbr = nbrLists[(idx, nm)][i]
          outL.append(nmDict[nbr[0]])
          outL.append('%.3f' % nbr[1])

        if outF:
          print(args.outputDelim.join(outL), file=outF)
  else:
    if not args.silent:
      logger.info('Creating output')
    curs = mConn.GetCursor()
    ids = [(x, ) for x in set(ids)]
    curs.execute('create temporary table _tmpTbl (%(idCol)s %(idTyp)s)' % locals())
    curs.executemany('insert into _tmpTbl values (?)', ids)
    molIdName = args.molIdName
    curs.execute('select %(idCol)s,%(molIdName)s from molecules join _tmpTbl using (%(idCol)s)' %
                 locals())
    nmDict = {}
    for guid, id in curs.fetchall():
      nmDict[guid] = str(id)
    if outF:
      print('\n'.join(nmDict.values()), file=outF)
  if molsOut and ids:
    molDbName = os.path.join(args.dbDir, args.molDbName)
    cns = [x.lower() for x in mConn.GetColumnNames('molecules')]
    if cns[-1] != 'molpkl':
      cns.remove('molpkl')
      cns.append('molpkl')

    curs = mConn.GetCursor()
    #curs.execute('create temporary table _tmpTbl (guid integer)'%locals())
    #curs.executemany('insert into _tmpTbl values (?)',ids)
    cnText = ','.join(cns)
    curs.execute('select %(cnText)s from molecules join _tmpTbl using (%(idCol)s)' % locals())

    row = curs.fetchone()
    molD = {}
    while row:
      row = list(row)
      m = _molFromPkl(row[-1])
      guid = row[0]
      nm = nmDict[guid]
      if sdfOut:
        m.SetProp('_Name', nm)
        print(Chem.MolToMolBlock(m), file=sdfOut)
        for i in range(1, len(cns) - 1):
          pn = cns[i]
          pv = str(row[i])
          print >> sdfOut, '> <%s>\n%s\n' % (pn, pv)
        print('$$$$', file=sdfOut)
      if smilesOut:
        smi = Chem.MolToSmiles(m, args.chiralSmiles)
      if smilesOut:
        print('%s %s' % (smi, str(row[1])), file=smilesOut)
      row = curs.fetchone()
  if not args.silent:
    logger.info('Done!')

# ---- ---- ---- ----  ---- ---- ---- ----  ---- ---- ---- ----  ---- ---- ---- ----
import os
import argparse

parser = argparse.ArgumentParser(usage)
parser.add_argument('-v', '--version', action='version', version='0.14.0')

parser.add_argument(
  '--dbDir', default='/db/camm/CURRENT/rdk_db',
  help='name of the directory containing the database information. The default is %(default)s')
parser.add_argument('--molDbName', default='Compounds.sqlt', help='name of the molecule database')
parser.add_argument('--molIdName', default='compound_id', help='name of the database key column')
parser.add_argument('--regName', default='molecules', help='name of the molecular registry table')
parser.add_argument('--pairDbName', default='AtomPairs.sqlt', help='name of the atom pairs database')
parser.add_argument('--pairTableName', default='atompairs', help='name of the atom pairs table')
parser.add_argument('--pairColName', default='atompairfp', help='name of the atom pair column')
parser.add_argument(
  '--torsionsDbName', default='AtomPairs.sqlt',
  help='name of the topological torsions database (usually the same as the atom pairs database)')
parser.add_argument(
  '--torsionsTableName', default='atompairs',
  help='name of the topological torsions table (usually the same as the atom pairs table)')
parser.add_argument('--torsionsColName', default='torsionfp', help='name of the atom pair column')
parser.add_argument('--fpDbName', default='Fingerprints.sqlt',
                  help='name of the 2D fingerprints database')
parser.add_argument('--fpTableName', default='rdkitfps', help='name of the 2D fingerprints table')
parser.add_argument('--layeredTableName', default='layeredfps',
                  help='name of the layered fingerprints table')
parser.add_argument('--fpColName', default='',
                  help='name of the 2D fingerprint column, a sensible default is used')
parser.add_argument('--descrDbName', default='Descriptors.sqlt',
                  help='name of the descriptor database')
parser.add_argument('--descrTableName', default='descriptors_v1', help='name of the descriptor table')
parser.add_argument('--descriptorCalcFilename', default=os.path.join(RDConfig.RDBaseDir, 'Projects',
                                                                   'DbCLI', 'moe_like.dsc'),
                  help='name of the file containing the descriptor calculator')
parser.add_argument('--outputDelim', default=',',
                  help='the delimiter for the output file. The default is %(default)s')
parser.add_argument(
  '--topN', default=20, type=int,
  help='the number of neighbors to keep for each query compound. The default is %(default)s')

parser.add_argument('--outF', '--outFile', default='-',
                  help='The name of the output file. The default is the console (stdout).')

parser.add_argument(
  '--transpose', default=False, action="store_true",
  help='print the results out in a transposed form: e.g. neighbors in rows and probe compounds in columns')

parser.add_argument('--molFormat', default='sdf', choices=('smiles', 'sdf'),
                  help='specify the format of the input file')
parser.add_argument(
  '--nameProp', default='_Name',
  help='specify the SD property to be used for the molecule names. Default is to use the mol block name')

parser.add_argument('--smartsQuery', '--smarts', '--sma', default='',
                  help='provide a SMARTS to be used as a substructure query')
parser.add_argument('--smilesQuery', '--smiles', '--smi', default='',
                  help='provide a SMILES to be used as a substructure query')
parser.add_argument('--negateQuery', '--negate', default=False, action='store_true',
                  help='negate the results of the smarts query.')
parser.add_argument('--propQuery', '--query', '-q', default='',
                  help='provide a property query (see the NOTE about property names)')

parser.add_argument('--sdfOut', '--sdOut', default='',
                  help='export an SD file with the matching molecules')
parser.add_argument('--smilesOut', '--smiOut', default='',
                  help='export a smiles file with the matching molecules')
parser.add_argument('--nonchiralSmiles', dest='chiralSmiles', default=True, action='store_false',
                  help='do not use chiral SMILES in the output')
parser.add_argument('--silent', default=False, action='store_true',
                  help='Do not generate status messages.')

parser.add_argument('--zipMols', '--zip', default=False, action='store_true',
                  help='read compressed mols from the database')

parser.add_argument('--pharm2DTableName', default='pharm2dfps',
                  help='name of the Pharm2D fingerprints table')
parser.add_argument('--fdefFile', '--fdef',
                  default=os.path.join(RDConfig.RDDataDir, 'Novartis1.fdef'),
                  help='provide the name of the fdef file to use for 2d pharmacophores')
parser.add_argument('--gobbi2DTableName', default='gobbi2dfps',
                  help='name of the Gobbi2D fingerprints table')

parser.add_argument(
  '--similarityType', '--simType', '--sim', default='RDK', choices=supportedSimilarityMethods,
  help='Choose the type of similarity to use, possible values: RDK, AtomPairs, TopologicalTorsions, Pharm2D, Gobbi2D, Avalon, Morgan. The default is %(default)s')
parser.add_argument('--morganFpDbName', default='Fingerprints.sqlt',
                  help='name of the morgan fingerprints database')
parser.add_argument('--morganFpTableName', default='morganfps',
                  help='name of the morgan fingerprints table')
parser.add_argument('--morganFpColName', default='morganfp',
                  help='name of the morgan fingerprint column')

parser.add_argument(
  '--similarityMetric', '--simMetric', '--metric', default='',
  choices=('tanimoto', 'dice', 'tversky', ''),
  help='Choose the type of similarity to use, possible values: tanimoto, dice, tversky. The default is determined by the fingerprint type')
parser.add_argument('--tverskyA', default=0.5, type=float, help='Tversky A value')
parser.add_argument('--tverskyB', default=0.5, type=float, help='Tversky B value')
parser.add_argument(
  '--simThresh', default=-1, type=float,
  help='threshold to use for similarity searching. If provided, this supersedes the topN argument')

if __name__ == '__main__':
  import sys, getopt, time

  args, args = parser.parse_args()
  if len(args) != 1 and not (args.smilesQuery or args.smartsQuery or args.propQuery):
    parser.error('please either provide a query filename argument or do a data or smarts query')

  if len(args):
    queryFilename = args[0]
  else:
    queryFilename = None
  args.queryMol = None
  RunSearch(args, queryFilename)
