# $Id$
#
#  Copyright (C) 2001-2006  greg Landrum
#
#   @@ All Rights Reserved  @@
#
"""basic unit testing code for the molecule boost wrapper

"""
from rdkit import RDConfig
import unittest,cPickle,os,time
from rdkit import Chem


class TestCase(unittest.TestCase):
  def setUp(self):
    self.bigSmiList = [
      "CC1=CC(=O)C=CC1=O",
      "S(SC1=NC2=CC=CC=C2S1)C3=NC4=C(S3)C=CC=C4",
      "OC1=C(Cl)C=C(C=C1[N+]([O-])=O)[N+]([O-])=O",
      "[O-][N+](=O)C1=CNC(=N)S1",
      "NC1=CC2=C(C=C1)C(=O)C3=C(C=CC=C3)C2=O",
      "OC(=O)C1=C(C=CC=C1)C2=C3C=CC(=O)C(=C3OC4=C2C=CC(=C4Br)O)Br",
      "CN(C)C1=C(Cl)C(=O)C2=C(C=CC=C2)C1=O",
      "CC1=C(C2=C(C=C1)C(=O)C3=CC=CC=C3C2=O)[N+]([O-])=O",
      "CC(=NO)C(C)=NO",
      "C1=CC=C(C=C1)P(C2=CC=CC=C2)C3=CC=CC=C3",
      "CC(C)(C)C1=C(O)C=C(C(=C1)O)C(C)(C)C",
      "CC1=NN(C(=O)C1)C2=CC=CC=C2",
      "NC1=CC=NC2=C1C=CC(=C2)Cl",
      "CCCCCC[CH]1CCCCN1",
      "O=CC1=C2C=CC=CC2=CC3=C1C=CC=C3",
      "BrN1C(=O)CCC1=O",
      "CCCCCCCCCCCCCCCC1=C(N)C=CC(=C1)O",
      "C(COC1=C(C=CC=C1)C2=CC=CC=C2)OC3=CC=CC=C3C4=CC=CC=C4",
      "CCCCSCC",
      "CC(=O)NC1=NC2=C(C=C1)C(=CC=N2)O",
      "CC1=C2C=CC(=NC2=NC(=C1)O)N",
      "CCOC(=O)C1=CN=C2N=C(N)C=CC2=C1O",
      "CC1=CC(=NC=C1)N=CC2=CC=CC=C2",
      "C[N+](C)(C)CC1=CC=CC=C1",
      "C[N+](C)(C)C(=O)C1=CC=CC=C1",
      "ICCC(C1=CC=CC=C1)(C2=CC=CC=C2)C3=CC=CC=C3",
      "CC1=CC(=C(C[N+](C)(C)C)C(=C1)C)C",
      "C[C](O)(CC(O)=O)C1=CC=C(C=C1)[N+]([O-])=O",
      "CC1=CC=C(C=C1)C(=O)C2=CC=C(Cl)C=C2",
      "ON=CC1=CC=C(O)C=C1",
      "CC1=CC(=C(N)C(=C1)C)C",
      "CC1=CC=C(C=C1)C(=O)C2=CC=C(C=C2)[N+]([O-])=O",
      "CC(O)(C1=CC=CC=C1)C2=CC=CC=C2",
      "ON=CC1=CC(=CC=C1)[N+]([O-])=O",
      "OC1=C2C=CC(=CC2=NC=C1[N+]([O-])=O)Cl",
      "CC1=CC=CC2=NC=C(C)C(=C12)Cl",
      "CCC(CC)([CH](OC(N)=O)C1=CC=CC=C1)C2=CC=CC=C2",
      "ON=C(CC1=CC=CC=C1)[CH](C#N)C2=CC=CC=C2",
      "O[CH](CC1=CC=CC=C1)C2=CC=CC=C2",
      "COC1=CC=C(CC2=CC=C(OC)C=C2)C=C1",
      "CN(C)[CH](C1=CC=CC=C1)C2=C(C)C=CC=C2",
      "COC1=CC(=C(N)C(=C1)[N+]([O-])=O)[N+]([O-])=O",
      "NN=C(C1=CC=CC=C1)C2=CC=CC=C2",
      "COC1=CC=C(C=C1)C=NO",
      "C1=CC=C(C=C1)C(N=C(C2=CC=CC=C2)C3=CC=CC=C3)C4=CC=CC=C4",
      "C1=CC=C(C=C1)N=C(C2=CC=CC=C2)C3=CC=CC=C3",
      "CC1=C(C2=CC=CC=C2)C(=C3C=CC=CC3=N1)O",
      "CCC1=[O+][Cu]2([O+]=C(CC)C1)[O+]=C(CC)CC(=[O+]2)CC",
      "OC(=O)[CH](CC1=CC=CC=C1)C2=CC=CC=C2",
      "CCC1=C(N)C=C(C)N=C1",
      ]


  def testPkl1(self):
    " testing single molecule pickle "
    m = Chem.MolFromSmiles('CCOC')
    outS = Chem.MolToSmiles(m)
    m2 = cPickle.loads(cPickle.dumps(m))
    outS2 = Chem.MolToSmiles(m2)
    assert outS==outS2,"bad pickle: %s != %s"%(outS,outS2)

  def testPkl2(self):
    """ further pickle tests """
    smis = self.bigSmiList
    for smi in smis:
      m = Chem.MolFromSmiles(smi)
      newM1 = cPickle.loads(cPickle.dumps(m))
      newM2 = cPickle.loads(cPickle.dumps(newM1))
      oldSmi = Chem.MolToSmiles(newM1)
      newSmi = Chem.MolToSmiles(newM2)
      assert newM1.GetNumAtoms()==m.GetNumAtoms(),'num atoms comparison failed'
      assert newM2.GetNumAtoms()==m.GetNumAtoms(),'num atoms comparison failed'
      assert oldSmi==newSmi,'string compare failed: %s != %s'%(oldSmi,newSmi)

  def testPkl(self):
    " testing molecule pickle "
    import tempfile
    f,self.fName = tempfile.mkstemp('.pkl')
    f=None
    self.m = Chem.MolFromSmiles('CC(=O)CC')
    cPickle.dump(self.m,open(self.fName,'wb+'))
    m2 = cPickle.load(open(self.fName,'rb'))
    try:
      os.unlink(self.fName)
    except:
      pass
    oldSmi = Chem.MolToSmiles(self.m)
    newSmi = Chem.MolToSmiles(m2)
    assert oldSmi==newSmi,'string compare failed'


  def testRings(self):
    " testing SSSR handling "
    m = Chem.MolFromSmiles('OC1C(O)C2C1C(O)C2O')
    for i in range(m.GetNumAtoms()):
      at = m.GetAtomWithIdx(i)
      n = at.GetAtomicNum()
      if n==8:
        assert not at.IsInRingSize(4),'atom %d improperly in ring'%(i)
      else:
        assert at.IsInRingSize(4),'atom %d not in ring of size 4'%(i)
if __name__ == '__main__':
  unittest.main()


