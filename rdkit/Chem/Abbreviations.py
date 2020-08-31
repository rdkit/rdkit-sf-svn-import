#
#  Copyright (C) 2020 Greg Landrum and T5 Informatics GmbH
#   @@ All Rights Reserved @@
#
#  This file is part of the RDKit.
#  The contents are covered by the terms of the BSD license
#  which is included in the file license.txt, found at the root
#  of the RDKit source tree.

#
from rdkit import Chem
import re

# ADAPTED FROM: https://github.com/openbabel/superatoms/blob/master/superatom.txt
defaultAbbreviations = '''
# Translations of superatom labels to SMILES.
# First atom of SMILES string should be the one connected to the rest of
# the molecule.
# Empty lines and lines starting with # are ignored.
# Originally from http://cactus.nci.nih.gov/osra/
# The left-aligned form is the one recognized in MDL alias lines;
# the right-aligned form may be used in 2D depiction.
# The whole list is used to look up alias names;
# only the part up to a line starting with ## is used to generate aliases.
# and here the largest fragments should be first; 
#left    right    SMILES		color
CO2Et    EtO2C    C(=O)OCC
COOEt    EtOOC    C(=O)OCC
OiBu     iBuO     OCC(C)C
tBu      tBu      C(C)(C)C
nBu      nBu      CCCC
iPr      iPr      C(C)C
nPr      nPr      CCC
Et       Et       CC
NCF3     F3CN     NC(F)(F)F
CF3      F3C      C(F)(F)F
CCl3     Cl3C     C(Cl)(Cl)Cl
CN       NC       C#N
NC       CN       [N+]#[C-]
N(OH)CH3 CH3(OH)N N(O)C
NO2      O2N      [N+](=O)[O-]
NO       ON       N=O
SO3H     HO3S     S(=O)(=O)O
CO2H     HOOC     C(=O)O		blue
COOH     HOOC     C(=O)O		blue
OEt      EtO      OCC
OAc      AcO      OC(=O)C
NHAc     AcNH     NC(=O)C
Ac       Ac       C(=O)C
CHO      OHC      C=O
NMe      MeN      NC
SMe      MeS      SC
OMe      MeO      OC
CO2-     -OOC     C(=O)[O-]
COO-     -OOC     C(=O)[O-]
'''


def isDummy(at):
  return at.GetAtomicNum() == 0 and \
      ((not at.HasQuery()) or at.DescribeQuery().strip() == 'AtomNull')


def preprocessAbbreviations(abbrevText=defaultAbbreviations):
  ls = [
    re.split(r'[\s]+', x) for x in abbrevText.split('\n') if (x and x[0] != '#' and x[0] != ' ')
  ]
  fgs = {}
  ps = Chem.AdjustQueryParameters()
  ps.adjustDegree = True
  ps.adjustDegreeFlags = Chem.AdjustQueryWhichFlags.ADJUST_IGNORENONE | Chem.AdjustQueryWhichFlags.ADJUST_IGNOREDUMMIES
  for l in ls:
    nm = l[0]
    smi = l[2]
    if not smi.startswith('*') and not smi.startswith('[*'):
      smi = '*' + smi
    q = Chem.MolFromSmiles(smi)

    nqAts = 0
    for at in q.GetAtoms():
      if not isDummy(at):
        nqAts += 1
    q._numNondummyAtoms = nqAts

    fgs[nm] = Chem.AdjustQueryProperties(q, ps)
  return fgs


def condense_abbreviation(mol, pattern, label, maxCoverage=0.4):
  '''

    maxCoverage idea borrowed from John Mayfield's CDK depictor. Set this >=1 to ignore it.
    The calculation ignores dummy atoms in the query

    doesn't do bonds between connected matches:
      condense_abbreviation(Chem.MolFromSmiles('c1ccccc1OCCOCCOc1ccncc1'),'OCC','CONN')

  '''
  if not mol.GetNumAtoms():
    return Chem.Mol(mol)
  if type(pattern) == Chem.Mol:
    query = pattern
  else:
    query = Chem.MolFromSmarts(pattern)

  a0 = query.GetAtomWithIdx(0)
  dummyAtFront = isDummy(a0)

  if maxCoverage > 0 and maxCoverage < 1:
    if not hasattr(query, '_numNondummyAtoms'):
      nQAts = 0
      for at in query.GetAtoms():
        if not isDummy(at):
          nQAts += 1
    else:
      nQAts = query._numNondummyAtoms
    if nQAts / mol.GetNumAtoms() >= maxCoverage:
      return Chem.Mol(mol)

  matches = mol.GetSubstructMatches(query)
  toRemove = []
  covered = set()
  res = Chem.RWMol(mol)
  for match in matches:
    if dummyAtFront:
      match = list(match)
      del match[0]
    if covered.intersection(set(match)):
      print("overlapping match ignored")
      continue
    covered.update(match)
    dummyIdx = res.AddAtom(Chem.Atom(0))
    needCoords = mol.GetNumConformers() > 0
    res.GetAtomWithIdx(dummyIdx).SetProp("atomLabel", label)
    for qidx, midx in enumerate(match):
      if needCoords:
        res.GetConformer().SetAtomPosition(dummyIdx, res.GetConformer().GetAtomPosition(midx))
        needCoords = False
      toRemove.append(midx)
      if mol.GetAtomWithIdx(midx).GetDegree() > query.GetAtomWithIdx(qidx).GetDegree():
        for at in mol.GetAtomWithIdx(midx).GetNeighbors():
          if at.GetIdx() not in match:
            res.AddBond(at.GetIdx(), dummyIdx, Chem.BondType.SINGLE)
  for idx in sorted(toRemove, reverse=True):
    res.RemoveAtom(idx)
  return res


###---------------------------------------
# Testing code
import unittest


class TestCase(unittest.TestCase):

  def setUp(self):
    self.defaultAbbrevs = preprocessAbbreviations()

  def testBasicsFromSmarts1(self):
    for ismi, osmi, sma, label in [
      ('c1ccccc1P(c1ccccc1)c1ccccc1', '*P(*)* |$Ph;;Ph;Ph$|', '*c1[cH][cH][cH][cH][cH]1', 'Ph'),
      ('C1CC1OCCC1CN1', '*(C1CC1)C1CN1 |$CONN;;;;;;$|', 'O!@[CH2]!@[CH2]', 'CONN')
        # problematic: ('C1CC1OCCC1CN1', 'C1CC1*C1CN1 ||', 'OCC', 'CONN')
    ]:
      m = Chem.MolFromSmiles(ismi)
      substm = condense_abbreviation(m, sma, label)
      smi = Chem.MolToCXSmiles(substm)
      self.assertEqual(osmi, smi)

  def testBasicsFromDefinitions(self):
    for ismi, osmi, label in [
      ('c1ccccc1OC', '*c1ccccc1 |$OMe;;;;;;$|', 'OMe'),
      ('c1ccccc1OCC', 'CCOc1ccccc1', 'OMe'),
      ('C1CC(C(F)(F)F)C1C(=O)O', '*C1CCC1C(F)(F)F |$COOH;;;;;;;;$|', 'COOH'),
      ('C1CC(C(F)(F)F)C1C(=O)O', '*C1CCC1C(F)(F)F |$CO2H;;;;;;;;$|', 'CO2H'),
      ('C1CC(C(F)(F)F)C1C(=O)O', '*C1CCC1C(=O)O |$CF3;;;;;;;$|', 'CF3'),
    ]:
      m = Chem.MolFromSmiles(ismi)
      substm = condense_abbreviation(m, self.defaultAbbrevs[label], label)
      smi = Chem.MolToCXSmiles(substm)
      self.assertEqual(osmi, smi)

  def testMaxCoverage(self):
    m = Chem.MolFromSmiles('CC(=O)O')
    substm = condense_abbreviation(m, self.defaultAbbrevs['CO2H'], 'CO2H')
    self.assertEqual(Chem.MolToCXSmiles(substm), 'CC(=O)O')
    substm = condense_abbreviation(m, self.defaultAbbrevs['CO2H'], 'CO2H', maxCoverage=1.0)
    self.assertEqual(Chem.MolToCXSmiles(substm), '*C |$CO2H;$|')


if __name__ == '__main__':  # pragma: nocover
  unittest.main()