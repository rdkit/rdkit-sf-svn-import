//
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
#include <GraphMol/RDKitQueries.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/SmilesParse/SmartsWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>

using namespace RDKit;

using matchCase = std::tuple<std::string, std::string, bool, bool>;

class _IsSubstructOf : public Catch::MatcherBase<const std::string &> {
  ROMol const *m_query;
  std::string m_description;
  SubstructMatchParameters m_ps;

 public:
  _IsSubstructOf(const ROMol &m, const std::string &description)
      : m_query(&m), m_description(description) {}

  _IsSubstructOf(const ROMol &m, const std::string &description,
                 SubstructMatchParameters ps)
      : m_query(&m), m_description(description), m_ps(ps) {}

  virtual bool match(const std::string &smiles) const override {
    std::unique_ptr<ROMol> mol(SmilesToMol(smiles));
    return !SubstructMatch(*mol, *m_query, m_ps).empty();
  }

  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "is not a substructure of " << m_description;
    return ss.str();
  }
};

static _IsSubstructOf IsSubstructOf(const ROMol &m, const std::string &smarts,
                                    const SubstructMatchParameters &ps) {
  return _IsSubstructOf(m, smarts, ps);
}

static _IsSubstructOf IsSubstructOf(const ROMol &m, const std::string &smarts) {
  return _IsSubstructOf(m, smarts);
}

TEST_CASE("handling of bondStereoCare in adjustQueryProperties") {
  SECTION("fully specified") {
    auto mol = R"CTAB(basic test
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0 STBOX=1
M  V30 2 C -5.6979 2.8332 0 0 STBOX=1
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2 STBOX=1
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB"_ctab;
    REQUIRE(mol);
    REQUIRE(mol->getBondBetweenAtoms(0, 1));
    CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
          Bond::BondStereo::STEREOE);
    MolOps::AdjustQueryParameters ps;
    ps.useStereoCareForBonds = true;
    MolOps::adjustQueryProperties(*mol, &ps);
    CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
          Bond::BondStereo::STEREOE);
  }
  SECTION("fully unspecified") {
    auto mol = R"CTAB(basic test
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0
M  V30 2 C -5.6979 2.8332 0 0
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB"_ctab;
    REQUIRE(mol);
    REQUIRE(mol->getBondBetweenAtoms(0, 1));
    CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
          Bond::BondStereo::STEREOE);
    MolOps::AdjustQueryParameters ps;
    ps.useStereoCareForBonds = true;
    MolOps::adjustQueryProperties(*mol, &ps);
    CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
          Bond::BondStereo::STEREONONE);
  }
  SECTION("partially unspecified") {
    std::vector<std::string> mbs = {R"CTAB(keep
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0 STBOX=1
M  V30 2 C -5.6979 2.8332 0 0 STBOX=1
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB",
                                    R"CTAB(keep
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0
M  V30 2 C -5.6979 2.8332 0 0
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2 STBOX=1
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB",
                                    R"CTAB(remove
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0
M  V30 2 C -5.6979 2.8332 0 0
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2 STBOX=0
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB",
                                    R"CTAB(remove
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0 
M  V30 2 C -5.6979 2.8332 0 0 STBOX=1
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB",
                                    R"CTAB(remove
  Mrv1810 01292006422D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 3 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -7.0316 2.0632 0 0 STBOX=1
M  V30 2 C -5.6979 2.8332 0 0
M  V30 3 O -4.3642 2.0632 0 0
M  V30 4 F -8.3653 2.8332 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 3
M  V30 2 1 1 4
M  V30 3 2 1 2
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB"};
    for (const auto &mb : mbs) {
      std::unique_ptr<RWMol> mol{MolBlockToMol(mb)};
      REQUIRE(mol);
      REQUIRE(mol->getBondBetweenAtoms(0, 1));
      CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
            Bond::BondStereo::STEREOE);
      MolOps::AdjustQueryParameters ps;
      ps.useStereoCareForBonds = true;
      MolOps::adjustQueryProperties(*mol, &ps);
      if (mol->getProp<std::string>(common_properties::_Name) == "keep") {
        CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
              Bond::BondStereo::STEREOE);
      } else {
        CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
              Bond::BondStereo::STEREONONE);
      }
    }
  }
  SECTION("V2000") {
    auto mol = R"CTAB(basic test
  Mrv1810 01292015042D          

  4  3  0  0  0  0            999 V2000
   -3.7669    1.1053    0.0000 C   0  0  0  0  1  0  0  0  0  0  0  0
   -3.0524    1.5178    0.0000 C   0  0  0  0  1  0  0  0  0  0  0  0
   -2.3380    1.1053    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -4.4814    1.5178    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  2  3  1  0  0  0  0
  1  4  1  0  0  0  0
  1  2  2  0  0  0  0
M  END
)CTAB"_ctab;
    REQUIRE(mol);
    CHECK(mol->getAtomWithIdx(0)->hasProp(common_properties::molStereoCare));
    CHECK(mol->getAtomWithIdx(1)->hasProp(common_properties::molStereoCare));
    REQUIRE(mol->getBondBetweenAtoms(0, 1));
    CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
          Bond::BondStereo::STEREOE);
    // property added by the CTAB parser:
    CHECK(mol->getBondBetweenAtoms(0, 1)->hasProp(
        common_properties::molStereoCare));
    MolOps::AdjustQueryParameters ps;
    ps.useStereoCareForBonds = true;
    MolOps::adjustQueryProperties(*mol, &ps);
    CHECK(mol->getBondBetweenAtoms(0, 1)->getStereo() ==
          Bond::BondStereo::STEREOE);
  }
  SECTION("molecule from SMILES") {
    auto mol = "C/C=C/C"_smiles;
    REQUIRE(mol);
    REQUIRE(mol->getBondBetweenAtoms(2, 1));
    CHECK(mol->getBondBetweenAtoms(2, 1)->getStereo() ==
          Bond::BondStereo::STEREOE);
    MolOps::AdjustQueryParameters ps;
    ps.useStereoCareForBonds = true;
    // since stereoCare is not set on the bond from SMILES,
    // stereochem will be removed:
    {
      RWMol molcp(*mol);
      MolOps::adjustQueryProperties(molcp, &ps);
      CHECK(molcp.getBondBetweenAtoms(2, 1)->getStereo() ==
            Bond::BondStereo::STEREONONE);
    }
    // but we can preserve it by setting the property:
    {
      RWMol molcp(*mol);
      molcp.getBondBetweenAtoms(2, 1)->setProp(common_properties::molStereoCare,
                                               1);
      MolOps::adjustQueryProperties(molcp, &ps);
      CHECK(molcp.getBondBetweenAtoms(2, 1)->getStereo() ==
            Bond::BondStereo::STEREOE);
    }
  }
}

TEST_CASE("adjustQueryParameters from JSON") {
  SECTION("basics") {
    MolOps::AdjustQueryParameters ps;
    CHECK(ps.makeAtomsGeneric == false);
    CHECK(ps.makeBondsGeneric == false);
    CHECK(ps.makeBondsGenericFlags == MolOps::ADJUST_IGNORENONE);

    std::string json = R"JSON({"makeAtomsGeneric":true})JSON";
    MolOps::parseAdjustQueryParametersFromJSON(ps, json);

    CHECK(ps.makeAtomsGeneric == true);
    CHECK(ps.makeBondsGeneric == false);
    // the parsing updates the parameters, it doesn't replace them:

    json = R"JSON({"makeBondsGeneric":true,
      "makeBondsGenericFlags":"IGNOREDUMMIES|IGNORECHAINS"})JSON";
    MolOps::parseAdjustQueryParametersFromJSON(ps, json);

    CHECK(ps.makeAtomsGeneric == true);
    CHECK(ps.makeBondsGeneric == true);
    CHECK(ps.makeBondsGenericFlags ==
          (MolOps::ADJUST_IGNOREDUMMIES | MolOps::ADJUST_IGNORECHAINS));
  }
  SECTION("useStereoCare") {
    MolOps::AdjustQueryParameters ps;
    CHECK(ps.useStereoCareForBonds == false);

    std::string json = R"JSON({"useStereoCareForBonds":true})JSON";
    MolOps::parseAdjustQueryParametersFromJSON(ps, json);
    CHECK(ps.useStereoCareForBonds == true);
    json = R"JSON({"useStereoCareForBonds":false})JSON";
    MolOps::parseAdjustQueryParametersFromJSON(ps, json);
    CHECK(ps.useStereoCareForBonds == false);
  }
  SECTION("bogus contents") {
    MolOps::AdjustQueryParameters ps;
    CHECK(ps.adjustDegree == true);
    CHECK(ps.adjustDegreeFlags ==
          (MolOps::ADJUST_IGNOREDUMMIES | MolOps::ADJUST_IGNORECHAINS));

    std::string json = R"JSON({"bogosity":true})JSON";
    MolOps::parseAdjustQueryParametersFromJSON(ps, json);
    CHECK(ps.adjustDegree == true);

    json = R"JSON({"adjustDegree":"foo"})JSON";
    MolOps::parseAdjustQueryParametersFromJSON(ps, json);
    CHECK(ps.adjustDegree == true);

    json = R"JSON({"adjustDegreeFlags":"IGNORENONE|bogus"})JSON";
    // clang-format off
    CHECK_THROWS_AS(MolOps::parseAdjustQueryParametersFromJSON(ps, json),ValueErrorException);
  }
}

TEST_CASE("five-rings") {
  MolOps::AdjustQueryParameters ps = MolOps::AdjustQueryParameters::noAdjustments();
  ps.adjustConjugatedFiveRings = true;
  SECTION("matching") {
    std::vector<matchCase> examples = {
    // 1,3 cyclopentadiene
    {"C1=CCC=C1","adjustqueryprops_fivering_1.mol",true,true},{"C1=CCC=C1","adjustqueryprops_fivering_2.mol",false,true},
    {"C1=CCC=C1","adjustqueryprops_fivering_3.mol",true,true},{"C1=CCC=C1","adjustqueryprops_fivering_4.mol",false,false},
    {"C1=CCC=C1","adjustqueryprops_fivering_5.mol",false,false},{"C1=CCC=C1","adjustqueryprops_fivering_6.mol",false,false},
    // pyrrole
    {"C1=CNC=C1","adjustqueryprops_fivering_1.mol",false,true},{"C1=CNC=C1","adjustqueryprops_fivering_2.mol",true,true},
    {"C1=CNC=C1","adjustqueryprops_fivering_3.mol",false,false},{"C1=CNC=C1","adjustqueryprops_fivering_4.mol",false,false},
    {"C1=CNC=C1","adjustqueryprops_fivering_5.mol",false,false},{"C1=CNC=C1","adjustqueryprops_fivering_6.mol",false,false},
    // thiophene
    {"C1=CSC=C1","adjustqueryprops_fivering_1.mol",false,false},{"C1=CSC=C1","adjustqueryprops_fivering_2.mol",true,true},
    {"C1=CSC=C1","adjustqueryprops_fivering_3.mol",false,false},{"C1=CSC=C1","adjustqueryprops_fivering_4.mol",true,true},
    {"C1=CSC=C1","adjustqueryprops_fivering_5.mol",false,false},{"C1=CSC=C1","adjustqueryprops_fivering_6.mol",true,true},
    // furan
    {"C1=COC=C1","adjustqueryprops_fivering_1.mol",false,true},{"C1=COC=C1","adjustqueryprops_fivering_2.mol",true,true},
    {"C1=COC=C1","adjustqueryprops_fivering_3.mol",false,false},{"C1=COC=C1","adjustqueryprops_fivering_4.mol",false,false},
    {"C1=COC=C1","adjustqueryprops_fivering_5.mol",false,false},{"C1=COC=C1","adjustqueryprops_fivering_6.mol",false,false},
    };
        for( const auto tpl : examples){
      auto fname = std::get<1>(tpl);
      std::string pathName = getenv("RDBASE");
      pathName += "/Code/GraphMol/test_data/";
      std::unique_ptr<RWMol> qry(MolFileToMol(pathName + fname));
      REQUIRE(qry);
      if(std::get<2>(tpl)){
        CHECK_THAT(std::get<0>(tpl),IsSubstructOf(*qry,fname));
      } else {
        CHECK_THAT(std::get<0>(tpl),!IsSubstructOf(*qry,fname));
      }
      MolOps::adjustQueryProperties(*qry,&ps);
      if(std::get<3>(tpl)){
        CHECK_THAT(std::get<0>(tpl),IsSubstructOf(*qry,fname));
      } else {
        CHECK_THAT(std::get<0>(tpl),!IsSubstructOf(*qry,fname));
      }
    } 
  }
  SECTION("query details") {
    auto fname = "adjustqueryprops_fivering_2.mol";
    std::string pathName = getenv("RDBASE");
    pathName += "/Code/GraphMol/test_data/";
    std::unique_ptr<RWMol> qry(MolFileToMol(pathName + fname));
    REQUIRE(qry);
    auto smarts = MolToSmarts(*qry);
    CHECK(smarts=="[!#1]1:[#6]:[#6]:[#6]:[#6]:1");
    MolOps::adjustQueryProperties(*qry,&ps);
    smarts = MolToSmarts(*qry);
    CHECK(smarts=="[!#1]1-,=,:[#6]-,=,:[#6]-,=,:[#6]-,=,:[#6]-,=,:1");
  }
}
