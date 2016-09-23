# $Id$
#
#  Copyright (C) 2007  greg Landrum
#
#   @@ All Rights Reserved @@
#  This file is part of the RDKit.
#  The contents are covered by the terms of the BSD license
#  which is included in the file license.txt, found at the root
#  of the RDKit source tree.
#
from __future__ import print_function
import unittest
import doctest
import os
import gzip
from rdkit.six.moves import cPickle  # @UnresolvedImport #pylint: disable=F0401
from rdkit import Chem
from rdkit import RDConfig
from rdkit.Chem.AtomPairs import Pairs, Torsions, Utils, Sheridan


def load_tests(loader, tests, ignore):  # pylint: disable=unused-argument
  """ Add the Doctests from the module """
  tests.addTests(doctest.DocTestSuite(Pairs, optionflags=doctest.ELLIPSIS))
  tests.addTests(doctest.DocTestSuite(Sheridan, optionflags=doctest.ELLIPSIS))
  tests.addTests(doctest.DocTestSuite(Torsions, optionflags=doctest.ELLIPSIS))
  tests.addTests(doctest.DocTestSuite(Utils, optionflags=doctest.ELLIPSIS))
  return tests


class TestCase(unittest.TestCase):

  def setUp(self):
    self.testDataPath = os.path.join(RDConfig.RDCodeDir, 'Chem', 'AtomPairs', 'test_data')
    inF = gzip.open(os.path.join(self.testDataPath, 'mols1000.pkl.gz'), 'rb')
    self.mols = cPickle.load(inF, encoding='bytes')

  def testPairsRegression(self):
    inF = gzip.open(os.path.join(self.testDataPath, 'mols1000.aps.pkl.gz'), 'rb')
    atomPairs = cPickle.load(inF, encoding='bytes')
    for i, m in enumerate(self.mols):
      ap = Pairs.GetAtomPairFingerprint(m)
      if ap != atomPairs[i]:  # pragma: nocover
        print(Chem.MolToSmiles(m))
        pd = ap.GetNonzeroElements()
        rd = atomPairs[i].GetNonzeroElements()
        for k, v in pd.items():
          if k in rd:
            if rd[k] != v:
              print('>>>1', k, v, rd[k])
          else:
            print('>>>2', k, v)
        for k, v in rd.items():
          if k in pd:
            if pd[k] != v:
              print('>>>3', k, v, pd[k])
          else:
            print('>>>4', k, v)
      self.assertEqual(ap, atomPairs[i])
      self.assertNotEqual(ap, atomPairs[i - 1])

  def testTorsionsRegression(self):
    inF = gzip.open(os.path.join(self.testDataPath, 'mols1000.tts.pkl.gz'), 'rb')
    torsions = cPickle.load(inF, encoding='bytes')
    for i, m in enumerate(self.mols):
      tt = Torsions.GetTopologicalTorsionFingerprintAsIntVect(m)
      if tt != torsions[i]:  # pragma: nocover
        print(Chem.MolToSmiles(m))
        pd = tt.GetNonzeroElements()
        rd = torsions[i].GetNonzeroElements()
        for k, v in pd.items():
          if k in rd:
            if rd[k] != v:
              print('>>>1', k, v, rd[k])
          else:
            print('>>>2', k, v)
        for k, v in rd.items():
          if k in pd:
            if pd[k] != v:
              print('>>>3', k, v, pd[k])
          else:
            print('>>>4', k, v)

      self.assertEqual(tt, torsions[i])
      self.assertNotEqual(tt, torsions[i - 1])

  def testGetTopologicalTorsionFingerprintAsIds(self):
    mol = Chem.MolFromSmiles('C1CCCCN1')
    tt = Torsions.GetTopologicalTorsionFingerprint(mol)
    self.assertEqual(tt.GetNonzeroElements(), {4437590049: 2, 8732557345: 2, 4445978657: 2})
    tt = Torsions.GetTopologicalTorsionFingerprintAsIds(mol)
    self.assertEqual(
      sorted(tt), [4437590049, 4437590049, 4445978657, 4445978657, 8732557345, 8732557345])
    tt = Torsions.GetTopologicalTorsionFingerprintAsIntVect(mol)
    self.assertEqual(tt.GetNonzeroElements(), {4437590049: 2, 8732557345: 2, 4445978657: 2})

  def testGithub334(self):
    m1 = Chem.MolFromSmiles('N#C')
    self.assertEqual(Utils.NumPiElectrons(m1.GetAtomWithIdx(0)), 2)
    self.assertEqual(Utils.NumPiElectrons(m1.GetAtomWithIdx(1)), 2)

    m1 = Chem.MolFromSmiles('N#[CH]')
    self.assertEqual(Utils.NumPiElectrons(m1.GetAtomWithIdx(0)), 2)
    self.assertEqual(Utils.NumPiElectrons(m1.GetAtomWithIdx(1)), 2)


if __name__ == '__main__':
  unittest.main()
