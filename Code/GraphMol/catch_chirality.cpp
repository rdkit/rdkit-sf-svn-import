//
//  Copyright (C) 2020 Greg Landrum and T5 Informatics GmbH
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#include "catch.hpp"

#include <GraphMol/RDKitBase.h>
#include <GraphMol/Chirality.h>
#include <GraphMol/MolOps.h>

#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>

using namespace RDKit;

TEST_CASE("bond StereoInfo", "[unittest]") {
  SECTION("basics") {
    {
      auto mol = "CC=C(C#C)C=C"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond_Double);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == Chirality::StereoInfo::NOATOM);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 5);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Unspecified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::None);
    }
    {
      auto mol = "CC=NC=N"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond_Double);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == Chirality::StereoInfo::NOATOM);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == Chirality::StereoInfo::NOATOM);
    }
  }
  SECTION("stereo") {
    {
      auto mol = "C/C=C(/C#C)C"_smiles;
      REQUIRE(mol);

      CHECK(mol->getBondWithIdx(1)->getStereoAtoms().size() == 2);
      CHECK(mol->getBondWithIdx(1)->getStereoAtoms()[0] == 0);
      CHECK(mol->getBondWithIdx(1)->getStereoAtoms()[1] == 3);

      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond_Double);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == Chirality::StereoInfo::NOATOM);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 5);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Specified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::Bond_Trans);
    }
    {  // check an example where one of the stereo atoms isn't the first
       // neighbor
      auto mol = "C/C=C(/C)C#C"_smiles;
      REQUIRE(mol);

      CHECK(mol->getBondWithIdx(1)->getStereoAtoms().size() == 2);
      CHECK(mol->getBondWithIdx(1)->getStereoAtoms()[0] == 0);
      CHECK(mol->getBondWithIdx(1)->getStereoAtoms()[1] == 4);

      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond_Double);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == Chirality::StereoInfo::NOATOM);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 4);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Specified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::Bond_Trans);
    }
    {
      auto mol = "C/C=C(\\C#C)C"_smiles;
      REQUIRE(mol);

      CHECK(mol->getBondWithIdx(1)->getStereoAtoms().size() == 2);
      CHECK(mol->getBondWithIdx(1)->getStereoAtoms()[0] == 0);
      CHECK(mol->getBondWithIdx(1)->getStereoAtoms()[1] == 3);

      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond_Double);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == Chirality::StereoInfo::NOATOM);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 5);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Specified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::Bond_Cis);
    }
    {  // any bonds
      auto mol = "CC=C(C#C)C"_smiles;
      REQUIRE(mol);

      mol->getBondWithIdx(1)->setStereo(Bond::BondStereo::STEREOANY);

      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond_Double);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == Chirality::StereoInfo::NOATOM);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 5);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Unknown);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::None);
    }
  }
}
TEST_CASE("isBondPotentialStereoBond", "[unittest]") {
  SECTION("basics") {
    {
      auto mol = "CC=C(C#C)C=C"_smiles;
      REQUIRE(mol);
      CHECK(
          Chirality::detail::isBondPotentialStereoBond(mol->getBondWithIdx(1)));
      CHECK(!Chirality::detail::isBondPotentialStereoBond(
          mol->getBondWithIdx(5)));
      CHECK(!Chirality::detail::isBondPotentialStereoBond(
          mol->getBondWithIdx(3)));
      CHECK(!Chirality::detail::isBondPotentialStereoBond(
          mol->getBondWithIdx(4)));
    }
    {
      auto mol = "CC=NC=N"_smiles;
      REQUIRE(mol);
      CHECK(
          Chirality::detail::isBondPotentialStereoBond(mol->getBondWithIdx(1)));
      CHECK(!Chirality::detail::isBondPotentialStereoBond(
          mol->getBondWithIdx(3)));
    }
    {
      SmilesParserParams ps;
      ps.removeHs = false;
      std::unique_ptr<ROMol> mol{SmilesToMol("[H]C=CC=C([H])[H]", ps)};
      REQUIRE(mol);
      CHECK(!Chirality::detail::isBondPotentialStereoBond(
          mol->getBondWithIdx(1)));
      CHECK(!Chirality::detail::isBondPotentialStereoBond(
          mol->getBondWithIdx(3)));
    }
  }
}

TEST_CASE("atom StereoInfo", "[unittest]") {
  SECTION("basics") {
    {
      auto mol = "CC(F)(Cl)CNC(C)C"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 2);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 4);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Unspecified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::None);

      sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(6));
      CHECK(sinfo.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(sinfo.centeredOn == 6);
      REQUIRE(sinfo.controllingAtoms.size() == 3);
      CHECK(sinfo.controllingAtoms[0] == 5);
      CHECK(sinfo.controllingAtoms[1] == 7);
      CHECK(sinfo.controllingAtoms[2] == 8);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Unspecified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::None);
    }

    {
      auto mol = "C[C@](F)(Cl)CNC(C)C"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 2);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 4);
      CHECK(sinfo.specified == Chirality::StereoSpecified::Specified);
      CHECK(sinfo.descriptor == Chirality::StereoDescriptor::Tet_CCW);
    }

    {
      auto mol = "CN1CC1N(F)C"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 3);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 2);
      CHECK(sinfo.controllingAtoms[2] == 3);
    }

    {
      auto mol = "O[As](F)C[As]C[As]"_smiles;
      REQUIRE(mol);

      auto sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 3);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 2);
      CHECK(sinfo.controllingAtoms[2] == 3);

      sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(4));
      CHECK(sinfo.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(sinfo.centeredOn == 4);
      REQUIRE(sinfo.controllingAtoms.size() == 2);
      CHECK(sinfo.controllingAtoms[0] == 3);
      CHECK(sinfo.controllingAtoms[1] == 5);
    }
  }
}
TEST_CASE("isAtomPotentialTetrahedralCenter", "[unittest]") {
  SECTION("basics") {
    {
      auto mol = "CC(F)(Cl)CNC(C)(C)C"_smiles;
      REQUIRE(mol);
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(1)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(0)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(4)));
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(6)));
    }
    {
      auto mol = "CN1CC1N(F)C"_smiles;
      REQUIRE(mol);
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(1)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(4)));
    }
    {
      auto mol = "O=S(F)CC[S+]([O-])CS=O"_smiles;
      REQUIRE(mol);
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(1)));
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(5)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(8)));
    }
    {
      auto mol = "O=[Se](F)CC[Se+]([O-])C[Se]=O"_smiles;
      REQUIRE(mol);
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(1)));
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(5)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(8)));
    }
    {
      auto mol = "OP(F)CPCP"_smiles;
      REQUIRE(mol);
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(1)));
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(4)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(6)));
    }
    {
      auto mol = "O[As](F)C[As]C[As]"_smiles;
      REQUIRE(mol);
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(1)));
      CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(4)));
      CHECK(!Chirality::detail::isAtomPotentialTetrahedralCenter(
          mol->getAtomWithIdx(6)));
    }
  }
}
TEST_CASE("isAtomPotentialStereoAtom", "[unittest]") {
  SECTION("basics") {
    {
      auto mol = "CC(F)(Cl)CNC(C)(C)C"_smiles;
      REQUIRE(mol);
      for (const auto atom : mol->atoms()) {
        CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(atom) ==
              Chirality::detail::isAtomPotentialStereoAtom(atom));
      }
    }
    {
      auto mol = "CN1CC1N(F)C"_smiles;
      REQUIRE(mol);
      for (const auto atom : mol->atoms()) {
        CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(atom) ==
              Chirality::detail::isAtomPotentialStereoAtom(atom));
      }
    }
    {
      auto mol = "O=S(F)CC[S+]([O-])CS=O"_smiles;
      REQUIRE(mol);
      for (const auto atom : mol->atoms()) {
        CHECK(Chirality::detail::isAtomPotentialTetrahedralCenter(atom) ==
              Chirality::detail::isAtomPotentialStereoAtom(atom));
      }
    }
  }
}

TEST_CASE("possible stereochemistry on atoms", "[chirality]") {
  SECTION("specified") {
    {
      auto mol = "CC(C)(O)[C@](Cl)(F)I"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 1);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[0].centeredOn == 4);
      std::vector<unsigned> catoms = {1, 5, 6, 7};
      CHECK(stereoInfo[0].controllingAtoms == catoms);
    }
    {
      auto mol = "C[C@@H](O)[C@H](C)[C@H](C)O"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 3);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[0].centeredOn == 1);

      CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[1].centeredOn == 3);

      CHECK(stereoInfo[2].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[2].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[2].centeredOn == 5);
    }

    {
      auto mol = "FC(F)(F)[C@@H](O)[C@H](C)[C@H](C(F)(F)F)O"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 3);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[0].centeredOn == 4);

      CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[1].centeredOn == 6);

      CHECK(stereoInfo[2].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[2].specified == Chirality::StereoSpecified::Specified);
      CHECK(stereoInfo[2].centeredOn == 8);
    }
  }

  SECTION("simple unspecified") {
    {
      auto mol = "CC(C)(O)C(Cl)(F)I"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 1);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Unspecified);
      CHECK(stereoInfo[0].centeredOn == 4);
      std::vector<unsigned> catoms = {1, 5, 6, 7};
      CHECK(stereoInfo[0].controllingAtoms == catoms);
    }
  }
}

TEST_CASE("possible stereochemistry on bonds", "[chirality]") {
  SECTION("simplest") {
    {
      auto mol = "CC=CC"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 1);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Bond_Double);
      CHECK(stereoInfo[0].centeredOn == 1);
      std::vector<unsigned> catoms = {0, Chirality::StereoInfo::NOATOM, 3,
                                      Chirality::StereoInfo::NOATOM};
      CHECK(stereoInfo[0].controllingAtoms == catoms);
    }
    {
      auto mol = "CC=C(C)C"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      CHECK(stereoInfo.size() == 0);
    }
    {
      auto mol = "CC=C"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      CHECK(stereoInfo.size() == 0);
    }
    {
      auto mol = "CC(F)=C(Cl)C"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 1);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Bond_Double);
      CHECK(stereoInfo[0].centeredOn == 2);
      std::vector<unsigned> catoms = {0, 2, 4, 5};
      CHECK(stereoInfo[0].controllingAtoms == catoms);
    }
    {
      auto mol = "CC=C(Cl)C"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 1);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Bond_Double);
      CHECK(stereoInfo[0].centeredOn == 1);
      std::vector<unsigned> catoms = {0, Chirality::StereoInfo::NOATOM, 3, 4};
      CHECK(stereoInfo[0].controllingAtoms == catoms);
    }
  }
}

TEST_CASE("para-stereocenters and assignStereochemistry", "[chirality]") {
  SECTION("simplest") {
    auto mol = "CC(F)C(C)C(C)F"_smiles;
    REQUIRE(mol);
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 3);

    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 1);
    CHECK(stereoInfo[0].controllingAtoms.size() == 3);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[1].centeredOn == 3);
    CHECK(stereoInfo[1].controllingAtoms.size() == 3);
    CHECK(stereoInfo[2].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[2].centeredOn == 5);
    CHECK(stereoInfo[2].controllingAtoms.size() == 3);
  }

  SECTION("including bonds") {
    // thanks to Salome Rieder for this nasty example
    auto mol = "CC=CC(C=CC)C(C)C(C=CC)C=CC"_smiles;
    REQUIRE(mol);
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    CHECK(stereoInfo.size() == 7);

    std::sort(stereoInfo.begin(), stereoInfo.end(),
              [](const Chirality::StereoInfo &a,
                 const Chirality::StereoInfo &b) -> bool {
                return (a.type > b.type) || (a.centeredOn > b.centeredOn) ||
                       (a.descriptor > b.descriptor) ||
                       (a.controllingAtoms > b.controllingAtoms);
              });
    REQUIRE(stereoInfo.size() == 7);

    CHECK(stereoInfo[0].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[0].centeredOn == 13);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[1].centeredOn == 10);
    CHECK(stereoInfo[2].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[2].centeredOn == 4);
    CHECK(stereoInfo[3].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[3].centeredOn == 1);

    CHECK(stereoInfo[4].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[4].centeredOn == 9);
    CHECK(stereoInfo[5].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[5].centeredOn == 7);
    CHECK(stereoInfo[6].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[6].centeredOn == 3);
  }
  SECTION("sugar fun") {
    auto mol = "C1(O)C(O)C(O)C(O)C(O)C1O"_smiles;
    REQUIRE(mol);
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 6);
    for (const auto &si : stereoInfo) {
      CHECK(si.type == Chirality::StereoType::Atom_Tetrahedral);
      CHECK(si.centeredOn % 2 == 0);
      CHECK(si.specified == Chirality::StereoSpecified::Unspecified);
    }
  }
}

TEST_CASE("ring stereochemistry", "[chirality]") {
  SECTION("specified") {
    auto mol = "C[C@H]1CC[C@@H](C)CC1"_smiles;
    REQUIRE(mol);
    // std::cerr << "------------ 1 -------------" << std::endl;

    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 1);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[1].centeredOn == 4);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Specified);
  }
#if 1
  // FIX this still isn't working
  SECTION("unspecified") {
    auto mol = "CC1CCC(C)CC1"_smiles;
    REQUIRE(mol);
    // std::cerr << "------------ 2 -------------" << std::endl;
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 1);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Unspecified);

    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[1].centeredOn == 4);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Unspecified);
  }
#endif
  SECTION("four ring") {
    auto mol = "C[C@H]1C[C@@H](C)C1"_smiles;
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 1);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);

    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[1].centeredOn == 3);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Specified);
  }
  SECTION("four ring unspecified") {
    auto mol = "CC1CC(C)C1"_smiles;
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 1);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Unspecified);

    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[1].centeredOn == 3);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Unspecified);
  }
}
#if 0
// FIX: the double bond stereo in rings isn't working
TEST_CASE("tricky recursive example from Dan Nealschneider", "[chirality]") {
  SECTION("adapted") {
    auto mol = "CC=C1CCC(O)CC1"_smiles;
    REQUIRE(mol);
    mol->updatePropertyCache();
    MolOps::setBondStereoFromDirections(*mol);
    std::cerr << "------------ 1 -------------" << std::endl;
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 5);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[1].centeredOn == 1);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Specified);
  }
  SECTION("simplified") {
    // can't sanitize this because the current (2020.03) assignStereochemistry
    // code doesn't recognize the stereo here and removes it
    SmilesParserParams ps;
    ps.sanitize = false;
    ps.removeHs = false;
    std::unique_ptr<ROMol> mol(SmilesToMol("C/C=C1/C[C@H](O)C1", ps));
    REQUIRE(mol);
    mol->updatePropertyCache();
    MolOps::setBondStereoFromDirections(*mol);
    std::cerr << "------------ 2 -------------" << std::endl;
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 4);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Specified);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[1].centeredOn == 1);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Specified);
  }
#if 1
  // FIX this still isn't working
  SECTION("unspecified") {
    auto mol = "CC=C1C[CH](O)C1"_smiles;
    REQUIRE(mol);
    std::cerr << "------------ 3 -------------" << std::endl;
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 2);
    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom_Tetrahedral);
    CHECK(stereoInfo[0].centeredOn == 4);
    CHECK(stereoInfo[0].specified == Chirality::StereoSpecified::Unspecified);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Bond_Double);
    CHECK(stereoInfo[1].centeredOn == 1);
    CHECK(stereoInfo[1].specified == Chirality::StereoSpecified::Unspecified);
  }
#endif
}
#endif