#
# Copyright (C) 2019 Greg Landrum and T5 Informatics GmbH
#  All Rights Reserved
#
#  This file is part of the RDKit.
#  The contents are covered by the terms of the BSD license
#  which is included in the file license.txt, found at the root
#  of the RDKit source tree.
#

from rdkit import Chem
from rdkit.Chem import rdChemReactions


class ScaffoldTreeParams(object):
  bondBreakers = ('[!#0;R:1]-!@[!#0:2]>>[*:1]-[#0].[#0]-[*:2]', )
  includeGenericScaffolds = True
  includeGenericBondScaffolds = False
  keepOnlyFirstFragment = True
  includeScaffoldsWithoutAttachments = True


def _addReactionsToParams(params):
  rxns = []
  for sma in params.bondBreakers:
    rxn = rdChemReactions.ReactionFromSmarts(sma)
    if rxn:
      rxns.append(rxn)
  params._breakers = tuple(rxns)


def _updateMolProps(m):
  ''' at the moment this just calls SanitizeMol.
  it's here in case we want to change that in the future
  '''
  Chem.SanitizeMol(m)


def getMolFragments(mol, params):
  """
    >>> ps = ScaffoldTreeParams()
    >>> m = Chem.MolFromSmiles('c1ccccc1CC1NC(=O)CCC1')
    >>> frags = getMolFragments(m,ps)
    >>> len(frags)
    2

    The results are 2-tuples with SMILES for the parent and then the fragment as a molecule:
    >>> frags[0]
    ('O=C1CCCC(Cc2ccccc2)N1', <rdkit.Chem.rdchem.Mol object at 0x...>)
    >>> sorted((x,Chem.MolToSmiles(y)) for x,y in frags)
    [('O=C1CCCC(Cc2ccccc2)N1', '*C1CCCC(=O)N1'), ('O=C1CCCC(Cc2ccccc2)N1', '*c1ccccc1')]

    Here's what the actual results look like:

    Setting keepOnlyFirstFragment results in us getting all the fragments with linkers:
    >>> ps.keepOnlyFirstFragment = False
    >>> frags = getMolFragments(m,ps)
    >>> len(frags)
    8
    >>> sorted((x,Chem.MolToSmiles(y)) for x,y in frags)
    [('*CC1CCCC(=O)N1', '*C*'), ('*CC1CCCC(=O)N1', '*C1CCCC(=O)N1'), ('*Cc1ccccc1', '*C*'), 
    ('*Cc1ccccc1', '*c1ccccc1'), ('O=C1CCCC(Cc2ccccc2)N1', '*C1CCCC(=O)N1'), ('O=C1CCCC(Cc2ccccc2)N1', 
    '*CC1CCCC(=O)N1'), ('O=C1CCCC(Cc2ccccc2)N1', '*Cc1ccccc1'), ('O=C1CCCC(Cc2ccccc2)N1', '*c1ccccc1')]

  """
  if not hasattr(params, '_breakers'):
    _addReactionsToParams(params)
  res = []
  stack = [mol]
  while stack:
    wmol = stack.pop(0)
    parent_smi = Chem.MolToSmiles(wmol)
    for rxn in params._breakers:
      ps = rxn.RunReactants((wmol, ))
      for p in ps:
        _updateMolProps(p[0])
        stack.append(p[0])
        res.append((parent_smi, p[0]))
        if not params.keepOnlyFirstFragment:
          _updateMolProps(p[1])
          stack.append(p[1])
          res.append((parent_smi, p[1]))
  return res


def removeAttachmentPoints(mol):
  """
  
  >>> m = Chem.MolFromSmiles('*c1ccc(*)cc1')
  >>> m.GetNumAtoms()
  8
  >>> sm = removeAttachmentPoints(m)
  >>> sm.GetNumAtoms()
  6
  >>> Chem.MolToSmiles(sm)
  'c1ccccc1'
  
  """
  res = Chem.RWMol(mol)
  aids = sorted(
    (x.GetIdx() for x in mol.GetAtoms() if x.GetAtomicNum() == 0 and x.GetDegree() == 1),
    reverse=True)
  for aid in aids:
    res.RemoveAtom(aid)
  return Chem.Mol(res)


def makeScaffoldGeneric(mol, doAtoms=True, doBonds=False):
  """

    >>> m = Chem.MolFromSmiles('*c1ncc(C(=O)O)cc1')
    >>> gm = makeScaffoldGeneric(m)
    >>> Chem.SanitizeMol(gm)
    rdkit.Chem.rdmolops.SanitizeFlags.SANITIZE_NONE
    >>> Chem.MolToSmiles(gm)
    '**(=*)*1:*:*:*(*):*:*:1'
    >>> gm2 = makeScaffoldGeneric(m,doBonds=True)
    >>> Chem.SanitizeMol(gm2)
    rdkit.Chem.rdmolops.SanitizeFlags.SANITIZE_NONE
    >>> Chem.MolToSmiles(gm2)
    '**1***(*(*)*)**1'
    >>> gm3 = makeScaffoldGeneric(m,doAtoms=False,doBonds=True)
    >>> Chem.SanitizeMol(gm3)
    rdkit.Chem.rdmolops.SanitizeFlags.SANITIZE_NONE
    >>> Chem.MolToSmiles(gm3)
    '*C1CCC(C(O)O)CN1'

    The original molecule is not affected:
    >>> Chem.MolToSmiles(m)
    '*c1ccc(C(=O)O)cn1'
    
  """
  res = Chem.Mol(mol)
  if doAtoms:
    for at in res.GetAtoms():
      at.SetAtomicNum(0)
  if doBonds:
    for bond in res.GetBonds():
      bond.SetBondType(Chem.BondType.SINGLE)
  return res


from collections import namedtuple
NetworkEdge = namedtuple('NetworkEdge', ('start', 'end', 'type'))


class EdgeTypes:
  FragmentEdge = 1
  GenericEdge = 2
  GenericBondEdge = 3
  RemoveAttachmentEdge = 4


def generateScaffoldNetwork(mol, params=None):
  """

  Run with default settings. The result is a 2-tuple with nodes and edges:
  >>> m = Chem.MolFromSmiles('c1ccccc1CC1NC(=O)CCC1')
  >>> nodes,edges = generateScaffoldNetwork(m)

  The nodes is a set of canonical SMILES describing the nodes:
  >>> len(nodes)
  9
  >>> nodes
  ['O=C1CCCC(Cc2ccccc2)N1', '*c1ccccc1', '**1:*:*:*:*:*:1', '*1:*:*:*:*:*:1', 
   'c1ccccc1', '*C1CCCC(=O)N1', '**1****(=*)*1', '*1*****1', 'O=C1CCCCN1']
  >>> len(edges)
  8
  >>> sorted(x for x in edges if x.type==EdgeTypes.FragmentEdge)          
  [NetworkEdge(start=0, end=1, type=1), 
   NetworkEdge(start=0, end=5, type=1)]
  >>> sorted(x for x in edges if x.type==EdgeTypes.GenericEdge)           
  [NetworkEdge(start=1, end=2, type=2), 
   NetworkEdge(start=5, end=6, type=2)]
  >>> sorted(x for x in edges if x.type==EdgeTypes.RemoveAttachmentEdge)  
  [NetworkEdge(start=1, end=4, type=4), 
   NetworkEdge(start=2, end=3, type=4), 
   NetworkEdge(start=5, end=8, type=4), 
   NetworkEdge(start=6, end=7, type=4)]

  """
  if params is None:
    params = ScaffoldTreeParams()
  nodes = []
  edges = []
  frags = getMolFragments(mol, params)
  for smi, fmol in frags:
    if smi not in nodes:
      nodes.append(smi)
    fsmi = Chem.MolToSmiles(fmol)
    if fsmi not in nodes:
      nodes.append(fsmi)
    edges.append(NetworkEdge(nodes.index(smi), nodes.index(fsmi), EdgeTypes.FragmentEdge))

    if params.includeGenericScaffolds:
      gmol = makeScaffoldGeneric(fmol, doAtoms=True, doBonds=False)
      gsmi = Chem.MolToSmiles(gmol)
      if gsmi not in nodes:
        nodes.append(gsmi)
      edges.append(NetworkEdge(nodes.index(fsmi), nodes.index(gsmi), EdgeTypes.GenericEdge))

      if params.includeScaffoldsWithoutAttachments:
        asmi = Chem.MolToSmiles(removeAttachmentPoints(gmol))
        if asmi not in nodes:
          nodes.append(asmi)
        edges.append(
          NetworkEdge(nodes.index(gsmi), nodes.index(asmi), EdgeTypes.RemoveAttachmentEdge))

      if params.includeGenericBondScaffolds:
        gbmol = makeScaffoldGeneric(fmol, doAtoms=True, doBonds=True)
        gbsmi = Chem.MolToSmiles(gbmol)
        if gbsmi not in nodes:
          nodes.append(gbsmi)
        edges.append(NetworkEdge(nodes.index(fsmi), nodes.index(gbsmi), EdgeTypes.GenericBondEdge))

        if params.includeScaffoldsWithoutAttachments:
          asmi = Chem.MolToSmiles(removeAttachmentPoints(gbmol))
          if asmi not in nodes:
            nodes.append(asmi)
          edges.append(
            NetworkEdge(nodes.index(gbsmi), nodes.index(asmi), EdgeTypes.RemoveAttachmentEdge))

    if params.includeScaffoldsWithoutAttachments:
      asmi = Chem.MolToSmiles(removeAttachmentPoints(fmol))
      if asmi not in nodes:
        nodes.append(asmi)
      edges.append(NetworkEdge(nodes.index(fsmi), nodes.index(asmi),
                               EdgeTypes.RemoveAttachmentEdge))

  return nodes, edges


# ------------------------------------
#
#  doctest boilerplate
#
def _runDoctests(verbose=None):  # pragma: nocover
  import sys
  import doctest
  failed, _ = doctest.testmod(optionflags=doctest.ELLIPSIS | doctest.NORMALIZE_WHITESPACE,
                              verbose=verbose)
  sys.exit(failed)


if __name__ == '__main__':  # pragma: nocover
  _runDoctests()
