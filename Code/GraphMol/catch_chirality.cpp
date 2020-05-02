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
      CHECK(sinfo.type == Chirality::StereoType::Bond);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 2);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 3);
    }
    {
      auto mol = "CC=NC=N"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getBondWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Bond);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 2);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 3);
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
      CHECK(sinfo.type == Chirality::StereoType::Atom);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 4);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 2);
      CHECK(sinfo.controllingAtoms[2] == 3);
      CHECK(sinfo.controllingAtoms[3] == 4);

      sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(6));
      CHECK(sinfo.type == Chirality::StereoType::Atom);
      CHECK(sinfo.centeredOn == 6);
      REQUIRE(sinfo.controllingAtoms.size() == 3);
      CHECK(sinfo.controllingAtoms[0] == 5);
      CHECK(sinfo.controllingAtoms[1] == 7);
      CHECK(sinfo.controllingAtoms[2] == 8);
    }

    {
      auto mol = "CN1CC1N(F)C"_smiles;
      REQUIRE(mol);
      auto sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(1));
      CHECK(sinfo.type == Chirality::StereoType::Atom);
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
      CHECK(sinfo.type == Chirality::StereoType::Atom);
      CHECK(sinfo.centeredOn == 1);
      REQUIRE(sinfo.controllingAtoms.size() == 3);
      CHECK(sinfo.controllingAtoms[0] == 0);
      CHECK(sinfo.controllingAtoms[1] == 2);
      CHECK(sinfo.controllingAtoms[2] == 3);

      sinfo = Chirality::detail::getStereoInfo(mol->getAtomWithIdx(4));
      CHECK(sinfo.type == Chirality::StereoType::Atom);
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

TEST_CASE("possible stereochemistry on bonds", "[chirality]") {
  SECTION("simplest") {
    {
      auto mol = "CC=CC"_smiles;
      REQUIRE(mol);
      auto stereoInfo = Chirality::findPotentialStereo(*mol);
      REQUIRE(stereoInfo.size() == 1);
      CHECK(stereoInfo[0].type == Chirality::StereoType::Bond);
      CHECK(stereoInfo[0].centeredOn == 1);
      REQUIRE(stereoInfo[0].controllingAtoms.size() == 2);
      CHECK(stereoInfo[0].controllingAtoms[0] == 0);
      CHECK(stereoInfo[0].controllingAtoms[1] == 3);
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
  }
}

TEST_CASE("para-stereocenters and assignStereochemistry", "[chirality]") {
  SECTION("simplest") {
    auto mol = "CC(F)CC(C)F"_smiles;
    REQUIRE(mol);
    auto stereoInfo = Chirality::findPotentialStereo(*mol);
    REQUIRE(stereoInfo.size() == 3);

    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom);
    CHECK(stereoInfo[0].centeredOn == 1);
    CHECK(stereoInfo[0].controllingAtoms.size() == 3);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom);
    CHECK(stereoInfo[1].centeredOn == 3);
    CHECK(stereoInfo[1].controllingAtoms.size() == 3);
    CHECK(stereoInfo[2].type == Chirality::StereoType::Atom);
    CHECK(stereoInfo[2].centeredOn == 4);
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

    CHECK(stereoInfo[0].type == Chirality::StereoType::Atom);
    CHECK(stereoInfo[0].centeredOn == 3);
    CHECK(stereoInfo[1].type == Chirality::StereoType::Atom);
    CHECK(stereoInfo[1].centeredOn == 7);
    CHECK(stereoInfo[2].type == Chirality::StereoType::Atom);
    CHECK(stereoInfo[2].centeredOn == 9);

    CHECK(stereoInfo[3].type == Chirality::StereoType::Bond);
    CHECK(stereoInfo[3].centeredOn == 1);
    CHECK(stereoInfo[4].type == Chirality::StereoType::Bond);
    CHECK(stereoInfo[4].centeredOn == 4);
    CHECK(stereoInfo[5].type == Chirality::StereoType::Bond);
    CHECK(stereoInfo[5].centeredOn == 9);
    CHECK(stereoInfo[6].type == Chirality::StereoType::Bond);
    CHECK(stereoInfo[6].centeredOn == 12);
  }
}
#if 0
TEST_CASE("possible stereochemistry on bonds", "[molops]") {
  SECTION("simplest") {
    {
      auto mol = "CC=CC"_smiles;
      REQUIRE(mol);
      MolOps::assignStereochemistry(*mol, cleanIt, force,
                                    flagPossibleStereoCenters);
      CHECK(mol->getBondWithIdx(1)->hasProp(
          common_properties::_ChiralityPossible));
    }
    {
      auto mol = "CC=C(C)C"_smiles;
      REQUIRE(mol);
      bool cleanIt = true;
      bool force = true;
      bool flagPossibleStereoCenters = true;
      MolOps::assignStereochemistry(*mol, cleanIt, force,
                                    flagPossibleStereoCenters);
      CHECK(!mol->getBondWithIdx(1)->hasProp(
          common_properties::_ChiralityPossible));
    }
    {
      auto mol = "CC=C"_smiles;
      REQUIRE(mol);
      bool cleanIt = true;
      bool force = true;
      bool flagPossibleStereoCenters = true;
      MolOps::assignStereochemistry(*mol, cleanIt, force,
                                    flagPossibleStereoCenters);
      CHECK(!mol->getBondWithIdx(1)->hasProp(
          common_properties::_ChiralityPossible));
    }
  }
}

TEST_CASE("para-stereocenters and assignStereochemistry", "[molops]") {
  SECTION("simplest") {
    auto mol = "CC(F)CC(C)F"_smiles;
    REQUIRE(mol);
    bool cleanIt = true;
    bool force = true;
    bool flagPossibleStereoCenters = true;
    MolOps::assignStereochemistry(*mol, cleanIt, force,
                                  flagPossibleStereoCenters);
    CHECK(
        mol->getAtomWithIdx(1)->hasProp(common_properties::_ChiralityPossible));
    CHECK(
        mol->getAtomWithIdx(4)->hasProp(common_properties::_ChiralityPossible));
    CHECK(
        mol->getAtomWithIdx(3)->hasProp(common_properties::_ChiralityPossible));
  }

  SECTION("including bonds") {
    // thanks to Salome Rieder for this nasty example
    auto mol = "CC=CC(C=CC)C(C)C(C=CC)C=CC"_smiles;
    REQUIRE(mol);
    CHECK(
        mol->getAtomWithIdx(7)->hasProp(common_properties::_ChiralityPossible));
    CHECK(
        mol->getAtomWithIdx(3)->hasProp(common_properties::_ChiralityPossible));
    CHECK(
        mol->getAtomWithIdx(9)->hasProp(common_properties::_ChiralityPossible));
    CHECK(mol->getBondBetweenAtoms(13, 14)->hasProp(
        common_properties::_ChiralityPossible));
    CHECK(mol->getBondBetweenAtoms(4, 5)->hasProp(
        common_properties::_ChiralityPossible));
    CHECK(mol->getBondBetweenAtoms(1, 2)->hasProp(
        common_properties::_ChiralityPossible));
    CHECK(mol->getBondBetweenAtoms(10, 11)->hasProp(
        common_properties::_ChiralityPossible));
  }
}
#endif
