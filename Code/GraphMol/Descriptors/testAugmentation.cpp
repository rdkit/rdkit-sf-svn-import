//  Created by Guillaume GODIN
//  Copyright (C) 2020 Greg Landrum
//   @@ All Rights Reserved @@
//
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.

#include <RDGeneral/Invariant.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <GraphMol/GraphMol.h>
#include <boost/tokenizer.hpp>
#include <GraphMol/Descriptors/Augmentation.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>


using namespace RDKit;

void testAugmentation(){
    std::cout << "===================== Testing Augmentation  =======================\n";
 
    auto mol = "C1CC=C1[NH3+]"_smiles; 
    unsigned int naug = 3;

    std::vector<std::string > res;

    RDKit::Descriptors::AugmentationVect(*mol, res, naug);

    for ( const auto &v : res ) {

      std::cout << v << "\n";

    }
    
    std::cout << "Aug test 1 Done\n";
}


int main() {
  RDLog::InitLogs();
  testAugmentation();
}