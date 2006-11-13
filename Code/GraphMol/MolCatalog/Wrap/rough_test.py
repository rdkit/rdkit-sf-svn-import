# $Id$
#
#  Copyright (C) 2006  Greg Landrum
#
import unittest,os,sys
import RDConfig
import Chem
import DataStructs
import cPickle
from Chem import MolCatalog

class TestCase(unittest.TestCase):
  def test1(self):
    cat = MolCatalog.CreateMolCatalog()
    es = []
    for smi in ('C1CCC1OC','C1CCC1','C'):
      m = Chem.MolFromSmiles(smi)
      entry = MolCatalog.MolCatalogEntry()
      entry.SetMol(m)
      self.failUnless(entry.GetMol())
      eSmi = Chem.MolToSmiles(entry.GetMol())
      self.failUnless(eSmi==Chem.MolToSmiles(m))
      entry.SetDescription(smi)
      self.failUnless(entry.GetDescription()==smi)
      es.append(entry)

    v=cat.AddEntry(es[0])
    self.failUnless(v==0)
    self.failUnless(cat.GetNumEntries()==1)

    v=cat.AddEntry(es[1])
    self.failUnless(v==1)
    self.failUnless(cat.GetNumEntries()==2)

    v=cat.AddEntry(es[2])
    self.failUnless(v==2)
    self.failUnless(cat.GetNumEntries()==3)

    cat.AddEdge(0,1)
    cat.AddEdge(0,2)
    cat.AddEdge(1,2)

    d = cPickle.dumps(cat)
    print >>sys.stderr,'es gone'
    es = None
    entry = None
    print >>sys.stderr,'cat gone'
    cat=None

    print >>sys.stderr,'reload'
    cat = cPickle.loads(d)
    self.failUnless(cat.GetNumEntries()==3)
    print >>sys.stderr,'cat gone'
    cat=None

    
     

    

if __name__ == '__main__':
    unittest.main()
