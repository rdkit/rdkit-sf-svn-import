//
//  Copyright (C) 2016 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <string>
#include <GraphMol/RDKitBase.h>
#include "SmilesParse.h"
#include "SmilesWrite.h"
#include <RDGeneral/RDLog.h>
using namespace RDKit;

void testBase() {
  BOOST_LOG(rdInfoLog) << "testing base functionality"
    << std::endl;
  { // it works when nothing is provided
    std::string smiles = "CC";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 2);
    delete m;
  }
  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}
void testCoords2D() {
  BOOST_LOG(rdInfoLog) << "testing reading 2D coordinates"
                       << std::endl;
  {
    std::string smiles = "CC |(0,.75,;0,-.75,)|";
    SmilesParserParams params;
    params.allowCXSMILES = true; 
    ROMol *m = SmilesToMol(smiles,params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms()==2);
    TEST_ASSERT(m->getNumConformers()==1);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(0).x ) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(0).y - 0.75) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(0).z ) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(1).x) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(1).y + 0.75) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(1).z) < 1e-4);

    delete m;
  }
  {
    std::string smiles = "CC |(,,;,,-.75)|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 2);
    TEST_ASSERT(m->getNumConformers() == 1);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(0).x) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(0).y) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(0).z) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(1).x) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(1).y) < 1e-4);
    TEST_ASSERT(fabs(m->getConformer().getAtomPos(1).z + 0.75) < 1e-4);

    delete m;
  }
  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

void testAtomLabels() {
  BOOST_LOG(rdInfoLog) << "testing reading Atom Labels"
    << std::endl;
  {
    std::string smiles = "CCC |$foo;;bar$|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getAtomWithIdx(0)->getProp<std::string>(common_properties::atomLabel) == "foo");
    TEST_ASSERT(m->getAtomWithIdx(2)->getProp<std::string>(common_properties::atomLabel) == "bar");
    TEST_ASSERT(!m->getAtomWithIdx(1)->hasProp(common_properties::atomLabel));
    delete m;
  }
  { // attachment points, example from the docs
    std::string smiles = "C[C@H](N*)C(*)=O |$;;;_AP1;;_AP2;$|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 7);
    TEST_ASSERT(m->getAtomWithIdx(3)->getAtomicNum() == 0);
    TEST_ASSERT(m->getAtomWithIdx(3)->getProp<std::string>(common_properties::atomLabel) == "_AP1");
    TEST_ASSERT(m->getAtomWithIdx(3)->getAtomMapNum() == 1);

    TEST_ASSERT(m->getAtomWithIdx(5)->getAtomicNum() == 0);
    TEST_ASSERT(m->getAtomWithIdx(5)->getProp<std::string>(common_properties::atomLabel) == "_AP2");
    TEST_ASSERT(m->getAtomWithIdx(5)->getAtomMapNum() == 2);
  }
  { // query properties
    std::string smiles = "**C |$Q_e;QH_p;;$|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getAtomWithIdx(0)->getProp<std::string>(common_properties::atomLabel) == "Q_e");
    TEST_ASSERT(m->getAtomWithIdx(1)->getProp<std::string>(common_properties::atomLabel) == "QH_p");
    TEST_ASSERT(!m->getAtomWithIdx(2)->hasProp(common_properties::atomLabel));
    TEST_ASSERT(m->getAtomWithIdx(0)->hasQuery());
    TEST_ASSERT(m->getAtomWithIdx(1)->hasQuery());
    TEST_ASSERT(!m->getAtomWithIdx(2)->hasQuery());

    delete m;
  }
  { // query properties2
    std::string smiles = "** |$;AH_p;$|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 2);
    TEST_ASSERT(m->getAtomWithIdx(1)->getProp<std::string>(common_properties::atomLabel) == "AH_p");
    TEST_ASSERT(!m->getAtomWithIdx(0)->hasProp(common_properties::atomLabel));
    TEST_ASSERT(m->getAtomWithIdx(0)->hasQuery());
    TEST_ASSERT(m->getAtomWithIdx(0)->getQuery()->getDescription() == "AtomAtomicNum");
    TEST_ASSERT(m->getAtomWithIdx(1)->hasQuery());
    TEST_ASSERT(m->getAtomWithIdx(1)->getQuery()->getDescription() == "AtomNull");

    delete m;
  }

  { // query properties3
    std::string smiles = "** |$;XH_p;$|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 2);
    TEST_ASSERT(m->getAtomWithIdx(1)->getProp<std::string>(common_properties::atomLabel) == "XH_p");
    TEST_ASSERT(!m->getAtomWithIdx(0)->hasProp(common_properties::atomLabel));
    TEST_ASSERT(m->getAtomWithIdx(0)->hasQuery());
    TEST_ASSERT(m->getAtomWithIdx(0)->getQuery()->getDescription()=="AtomAtomicNum");
    TEST_ASSERT(m->getAtomWithIdx(1)->hasQuery());
    TEST_ASSERT(m->getAtomWithIdx(1)->getQuery()->getDescription() == "AtomOr");

    delete m;
  }
  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

void testCXSmilesAndName() {
  BOOST_LOG(rdInfoLog) << "testing CSXMILES and mol name"
    << std::endl;
  {
    std::string smiles = "CCC |$foo;;bar$|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    params.parseName = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getAtomWithIdx(0)->getProp<std::string>(common_properties::atomLabel) == "foo");
    TEST_ASSERT(!m->hasProp("_Name"));
    delete m;
  }
  {
    std::string smiles = "CCC |$foo;;bar$| ourname";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    params.parseName = true;

    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getAtomWithIdx(0)->getProp<std::string>(common_properties::atomLabel) == "foo");
    TEST_ASSERT(m->getProp<std::string>("_Name")=="ourname");
    delete m;
  }
  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}


void testCoordinateBonds() {
  BOOST_LOG(rdInfoLog) << "testing coordinate bonds" << std::endl;
  {
    std::string smiles = "[Fe]1C=C1 |C:1.0,2.2|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getBondBetweenAtoms(1,2));
    TEST_ASSERT(m->getBondBetweenAtoms(1,2)->getBondType() == Bond::DOUBLE);
    TEST_ASSERT(m->getBondBetweenAtoms(0,1));
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1)->getBondType()==Bond::DATIVE);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1)->getBeginAtomIdx() == 1);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 2));
    TEST_ASSERT(m->getBondBetweenAtoms(0, 2)->getBondType() == Bond::DATIVE);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 2)->getBeginAtomIdx() == 2);
    delete m;
  }
  {
    std::string smiles = "C1[Fe]C=1 |C:0.0,2.1|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 2));
    TEST_ASSERT(m->getBondBetweenAtoms(0, 2)->getBondType() == Bond::DOUBLE);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1));
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1)->getBondType() == Bond::DATIVE);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1)->getBeginAtomIdx() == 0);
    TEST_ASSERT(m->getBondBetweenAtoms(1, 2));
    TEST_ASSERT(m->getBondBetweenAtoms(1, 2)->getBondType() == Bond::DATIVE);
    TEST_ASSERT(m->getBondBetweenAtoms(1, 2)->getBeginAtomIdx() == 2);
    delete m;
  }
  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}


void testRadicals() {
  BOOST_LOG(rdInfoLog) << "testing radicals" << std::endl;
  {
    std::string smiles = "[O]C[O] |^1:0,2|";      ;
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getAtomWithIdx(0)->getNumRadicalElectrons() == 1);
    TEST_ASSERT(m->getAtomWithIdx(1)->getNumRadicalElectrons() == 0);
    TEST_ASSERT(m->getAtomWithIdx(2)->getNumRadicalElectrons() == 1);

    delete m;
  }
  {
    std::string smiles = "[O][C][O] |^1:0,2,^4:1|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 3);
    TEST_ASSERT(m->getAtomWithIdx(0)->getNumRadicalElectrons() == 1);
    TEST_ASSERT(m->getAtomWithIdx(1)->getNumRadicalElectrons() == 2);
    TEST_ASSERT(m->getAtomWithIdx(2)->getNumRadicalElectrons() == 1);

    delete m;
  }
  { // radicals and coordinate bonds
    std::string smiles = "[Fe]N([O])[O] |^1:2,3,C:1.0|";
    SmilesParserParams params;
    params.allowCXSMILES = true;
    ROMol *m = SmilesToMol(smiles, params);
    TEST_ASSERT(m);
    TEST_ASSERT(m->getNumAtoms() == 4);
    TEST_ASSERT(m->getAtomWithIdx(1)->getNumRadicalElectrons() == 0);
    TEST_ASSERT(m->getAtomWithIdx(2)->getNumRadicalElectrons() == 1);
    TEST_ASSERT(m->getAtomWithIdx(3)->getNumRadicalElectrons() == 1);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1));
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1)->getBondType() == Bond::DATIVE);
    TEST_ASSERT(m->getBondBetweenAtoms(0, 1)->getBeginAtomIdx() == 1);

    delete m;
  }

  
    BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  RDLog::InitLogs();
  testBase();
  testCoords2D();
  testAtomLabels();
  testCXSmilesAndName();
  testCoordinateBonds();
  testRadicals();

}
