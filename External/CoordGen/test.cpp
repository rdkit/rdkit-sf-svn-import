//
//  Copyright (C) 2017-2020 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <iostream>

#include <fstream>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/MolAlign/AlignMolecules.h>
#include <GraphMol/MolTransforms/MolTransforms.h>

#include <RDGeneral/RDLog.h>

#include "coordgen/sketcherMinimizer.h"
#include <CoordGen/CoordGen.h>

using namespace RDKit;

void test1() {
  BOOST_LOG(rdInfoLog) << "-------------------------------------" << std::endl;
  BOOST_LOG(rdInfoLog) << "test1: basics" << std::endl;
#if 1
  {
    ROMol* m = SmilesToMol("c1cc(CC)cnc1CC(=O)O");
    TEST_ASSERT(m);
    m->setProp("_Name", "test1");

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    auto mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    delete m;
  }
  {
    // ROMol* m = SmilesToMol("c1ccncc1");

    ROMol* m = SmilesToMol("ClC(O)(F)C");
    TEST_ASSERT(m);
    m->setProp("_Name", "test2");

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    auto mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    delete m;
  }

  {
    ROMol* m = SmilesToMol(
        "CC[C@H]1C(=O)N(CC(=O)N([C@H](C(=O)N[C@H](C(=O)N([C@H](C(=O)N[C@H](C(="
        "O)N[C@@H](C(=O)N([C@H](C(=O)N([C@H](C(=O)N([C@H](C(=O)N([C@H](C(=O)N1)"
        "[C@@H]([C@H](C)C/C=C/"
        "C)O)C)C(C)C)C)CC(C)C)C)CC(C)C)C)C)C)CC(C)C)C)C(C)C)CC(C)C)C)C");
    TEST_ASSERT(m);
    m->setProp("_Name", "cyclosporine a");

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    auto mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    delete m;
  }

  {
    // ROMol* m = SmilesToMol("c1ccncc1");

    ROMol* m = SmilesToMol("CCCNC=CNCOC=CC=CC=COC");
    TEST_ASSERT(m);
    m->setProp("_Name", "single-double");

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    auto mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    delete m;
  }
#endif

  {
    ROMol* m = SmilesToMol("O/C=C/C=C/C=C\\C=C/N");
    TEST_ASSERT(m);
    m->setProp("_Name", "cis-trans");

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    auto mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    delete m;
  }

  {
    ROMol* m = SmilesToMol("C1C3CC2CC(CC1C2)C3");
    TEST_ASSERT(m);
    m->setProp("_Name", "admntn");

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    auto mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    delete m;
  }

  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

namespace {
bool compareConfs(const ROMol* m, ROMol* templ, const MatchVectType& mv,
                  bool alignFirst = false, int molConfId = -1,
                  int templateConfId = -1, double postol = 1e-2,
                  double rmstol = 0.1) {
  PRECONDITION(m, "bad pointer");
  PRECONDITION(templ, "bad pointer");
  TEST_ASSERT(m->getNumAtoms() >= templ->getNumAtoms());

  if (alignFirst) {
    double rmsd =
        MolAlign::alignMol(*templ, *m, molConfId, templateConfId, &mv);
    if (rmsd > rmstol) {
      return false;
    }
  }

  const Conformer& conf1 = m->getConformer(molConfId);
  const Conformer& conf2 = templ->getConformer(templateConfId);
  for (unsigned int i = 0; i < templ->getNumAtoms(); i++) {
    TEST_ASSERT(m->getAtomWithIdx(mv[i].second)->getAtomicNum() ==
                templ->getAtomWithIdx(mv[i].first)->getAtomicNum());

    RDGeom::Point3D pt1i = conf1.getAtomPos(mv[i].second);
    RDGeom::Point3D pt2i = conf2.getAtomPos(mv[i].first);
    if ((pt1i - pt2i).length() >= postol) {
      return false;
    }
  }
  return true;
}
}  // namespace

void test2() {
  BOOST_LOG(rdInfoLog) << "-------------------------------------" << std::endl;
  BOOST_LOG(rdInfoLog) << "test2: using templates" << std::endl;

  {
    ROMol* core = SmilesToMol("C1CON1");
    TEST_ASSERT(core);
    core->setProp("_Name", "core");

    TEST_ASSERT(CoordGen::addCoords(*core) == 0);
    TEST_ASSERT(core->getNumConformers() == 1);
    auto mb = MolToMolBlock(*core);
    std::cerr << mb << std::endl;

    ROMol* m = SmilesToMol("C1C(CCC)ON1");
    TEST_ASSERT(m);
    m->setProp("_Name", "core+sidechain");

    MatchVectType mv;
    SubstructMatch(*m, *core, mv);

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    TEST_ASSERT(!compareConfs(m, core, mv));

    {
      auto coreConf = core->getConformer();
      RDGeom::INT_POINT2D_MAP coordMap;
      for (auto& i : mv) {
        coordMap[i.second] = RDGeom::Point2D(coreConf.getAtomPos(i.first).x,
                                             coreConf.getAtomPos(i.first).y);
      }
      CoordGen::CoordGenParams params;
      params.coordMap = coordMap;
      params.dbg_useFixed = true;
      TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
      TEST_ASSERT(m->getNumConformers() == 1);
      // m->setProp("_Name", "templated");
      // mb = MolToMolBlock(*m);
      // std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv));
    }
    {
      CoordGen::CoordGenParams params;
      params.templateMol = core;
      params.dbg_useFixed = true;
      TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
      TEST_ASSERT(m->getNumConformers() == 1);
      m->setProp("_Name", "templated");
      mb = MolToMolBlock(*m);
      std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv));
    }
    delete m;
    delete core;
  }

  {
    ROMol* core = SmilesToMol("C1CCCCCONCN1");
    TEST_ASSERT(core);
    core->setProp("_Name", "core");

    TEST_ASSERT(CoordGen::addCoords(*core) == 0);
    TEST_ASSERT(core->getNumConformers() == 1);
    auto mb = MolToMolBlock(*core);
    std::cerr << mb << std::endl;

    ROMol* m = SmilesToMol("C1CCCCONC(CC)NC1");
    TEST_ASSERT(m);
    m->setProp("_Name", "core+sidechain");

    MatchVectType mv;
    SubstructMatch(*m, *core, mv);

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    TEST_ASSERT(!compareConfs(m, core, mv));

    {
      auto coreConf = core->getConformer();
      RDGeom::INT_POINT2D_MAP coordMap;
      for (auto& i : mv) {
        coordMap[i.second] = RDGeom::Point2D(coreConf.getAtomPos(i.first).x,
                                             coreConf.getAtomPos(i.first).y);
      }
      CoordGen::CoordGenParams params;
      params.coordMap = coordMap;
      params.dbg_useFixed = true;
      TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
      TEST_ASSERT(m->getNumConformers() == 1);
      // m->setProp("_Name", "templated");
      // mb = MolToMolBlock(*m);
      // std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv));
    }
    {
      CoordGen::CoordGenParams params;
      params.templateMol = core;
      params.dbg_useFixed = true;
      TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
      TEST_ASSERT(m->getNumConformers() == 1);
      m->setProp("_Name", "templated");
      mb = MolToMolBlock(*m);
      std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv));
    }

    delete m;
    delete core;
  }

  {
    ROMol* core = SmilesToMol("C1CCCCCONCN1");
    TEST_ASSERT(core);
    core->setProp("_Name", "core");

    TEST_ASSERT(CoordGen::addCoords(*core) == 0);
    TEST_ASSERT(core->getNumConformers() == 1);
    auto mb = MolToMolBlock(*core);
    std::cerr << mb << std::endl;

    ROMol* m = SmilesToMol("C1CCCCONC(CCCCCC)NC1");
    TEST_ASSERT(m);
    m->setProp("_Name", "core+sidechain");

    MatchVectType mv;
    SubstructMatch(*m, *core, mv);

    TEST_ASSERT(CoordGen::addCoords(*m) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    TEST_ASSERT(!compareConfs(m, core, mv));

    {
      auto coreConf = core->getConformer();
      RDGeom::INT_POINT2D_MAP coordMap;
      for (auto& i : mv) {
        coordMap[i.second] = RDGeom::Point2D(coreConf.getAtomPos(i.first).x,
                                             coreConf.getAtomPos(i.first).y);
      }

      CoordGen::CoordGenParams params;
      params.coordMap = coordMap;
      params.dbg_useFixed = true;
      TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
      TEST_ASSERT(m->getNumConformers() == 1);
      // m->setProp("_Name", "templated");
      // mb = MolToMolBlock(*m);
      // std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv));
    }
    {
      CoordGen::CoordGenParams params;
      params.templateMol = core;
      params.dbg_useFixed = true;
      TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
      TEST_ASSERT(m->getNumConformers() == 1);
      m->setProp("_Name", "templated");
      mb = MolToMolBlock(*m);
      std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv));
    }
    delete m;
    delete core;
  }

  {
    ROMol* core = SmilesToMol("C1CCCC2C1NCC2");
    TEST_ASSERT(core);
    core->setProp("_Name", "core");

    CoordGen::addCoords(*core);
    TEST_ASSERT(core->getNumConformers() == 1);
    auto mb = MolToMolBlock(*core);
    std::cerr << mb << std::endl;

    ROMol* m = SmilesToMol("C1C(CCC)CC(CC3CC3)C2C1N(C(C)C)CC2");
    TEST_ASSERT(m);
    m->setProp("_Name", "core+sidechain");

    MatchVectType mv;
    SubstructMatch(*m, *core, mv);

    CoordGen::addCoords(*m);
    TEST_ASSERT(m->getNumConformers() == 1);
    mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    // This is a rigid core: if we provide the matching substructure,
    // and do alignment, the conformations will still match.
    TEST_ASSERT(!compareConfs(m, core, mv, false));

    {
      CoordGen::CoordGenParams params;
      params.templateMol = core;
      CoordGen::addCoords(*m, &params);
      TEST_ASSERT(m->getNumConformers() == 1);
      m->setProp("_Name", "templated");
      mb = MolToMolBlock(*m);
      std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv, true, -1, -1, 0.3));
    }
    delete m;
    delete core;
  }

  {
    ROMol* core = SmilesToMol("CC(N)CC");
    TEST_ASSERT(core);
    core->setProp("_Name", "core");

    CoordGen::addCoords(*core);
    TEST_ASSERT(core->getNumConformers() == 1);
    auto mb = MolToMolBlock(*core);
    std::cerr << mb << std::endl;

    ROMol* m = SmilesToMol("CC(N)CC(O)C");
    TEST_ASSERT(m);
    m->setProp("_Name", "core+sidechain");

    MatchVectType mv;
    SubstructMatch(*m, *core, mv);

    CoordGen::addCoords(*m);
    TEST_ASSERT(m->getNumConformers() == 1);
    mb = MolToMolBlock(*m);
    std::cerr << mb << std::endl;
    // This mol is just slightly bigger than the core, providing
    // the matching substructure and doing alignment will cause
    // the conformations to match.
    TEST_ASSERT(!compareConfs(m, core, mv, false));

    {
      CoordGen::CoordGenParams params;
      params.templateMol = core;
      CoordGen::addCoords(*m, &params);
      TEST_ASSERT(m->getNumConformers() == 1);
      m->setProp("_Name", "templated");
      mb = MolToMolBlock(*m);
      std::cerr << mb << std::endl;
      TEST_ASSERT(compareConfs(m, core, mv, true, -1, -1, 0.05));
    }
    delete m;
    delete core;
  }

  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

void testGithub1929() {
  BOOST_LOG(rdInfoLog) << "-------------------------------------" << std::endl;
  BOOST_LOG(rdInfoLog)
      << "testing github1929: make sure coordgen works with bogus file names"
      << std::endl;
  {
    ROMol* m = SmilesToMol("c1cc(CC)cnc1CC(=O)O");
    TEST_ASSERT(m);
    m->setProp("_Name", "test1");
    CoordGen::CoordGenParams params;
    params.templateFileDir = "I_do_not_exist";

    TEST_ASSERT(CoordGen::addCoords(*m, &params) == 0);
    TEST_ASSERT(m->getNumConformers() == 1);
    delete m;
  }

  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

void testGithub3131() {
  BOOST_LOG(rdInfoLog) << "-------------------------------------" << std::endl;
  BOOST_LOG(rdInfoLog)
      << "testing github3131: results from coordgen are sometimes not centered"
      << std::endl;
  {
    auto m1 =
        "CC1=C(C=C(C=C1)NC(=O)C2=CC=C(C=C2)CN3CCN(CC3)C)NC4=NC=CC(=N4)C5=CN=CC="
        "C5"_smiles;
    TEST_ASSERT(m1);
    TEST_ASSERT(CoordGen::addCoords(*m1) == 0);
    TEST_ASSERT(m1->getNumConformers() == 1);
    auto center = MolTransforms::computeCentroid(m1->getConformer());
    TEST_ASSERT(feq(center.x, 0.0));
    TEST_ASSERT(feq(center.y, 0.0));
  }

  {
    auto m1 =
        "CCC1=C2N=C(C=C(N2N=C1)NCC3=C[N+](=CC=C3)[O-])N4CCCC[C@H]4CCO"_smiles;
    TEST_ASSERT(m1);
    TEST_ASSERT(CoordGen::addCoords(*m1) == 0);
    TEST_ASSERT(m1->getNumConformers() == 1);
    auto center = MolTransforms::computeCentroid(m1->getConformer());
    TEST_ASSERT(feq(center.x, 0.0));
    TEST_ASSERT(feq(center.y, 0.0));
  }

  {
    // make sure that it's not recentered if we provide a coordmap:
    auto m1 =
        "CCC1=C2N=C(C=C(N2N=C1)NCC3=C[N+](=CC=C3)[O-])N4CCCC[C@H]4CCO"_smiles;
    TEST_ASSERT(m1);
    CoordGen::CoordGenParams params;
    params.coordMap[0] = {10.0, 10.0};
    params.coordMap[1] = {11.0, 10.0};
    TEST_ASSERT(CoordGen::addCoords(*m1, &params) == 0);
    TEST_ASSERT(m1->getNumConformers() == 1);
    auto center = MolTransforms::computeCentroid(m1->getConformer());
    TEST_ASSERT(!feq(center.x, 0.0));
    TEST_ASSERT(!feq(center.y, 0.0));
  }

  {
    // make sure that it's not recentered if we provide a template:
    auto templateMol =
        "C1=C2N=C(C=C(N2N=C1)NCC3=C[N+](=CC=C3))N4CCCC[C@H]4"_smiles;
    TEST_ASSERT(templateMol);
    TEST_ASSERT(CoordGen::addCoords(*templateMol) == 0);
    TEST_ASSERT(templateMol->getNumConformers() == 1);

    auto center = MolTransforms::computeCentroid(templateMol->getConformer());
    TEST_ASSERT(feq(center.x, 0.0));
    TEST_ASSERT(feq(center.y, 0.0));

    auto m1 =
        "CCC1=C2N=C(C=C(N2N=C1)NCC3=C[N+](=CC=C3)[O-])N4CCCC[C@H]4CCO"_smiles;
    TEST_ASSERT(m1);
    CoordGen::CoordGenParams params;
    params.templateMol = templateMol.get();
    TEST_ASSERT(CoordGen::addCoords(*m1, &params) == 0);
    TEST_ASSERT(m1->getNumConformers() == 1);
    center = MolTransforms::computeCentroid(m1->getConformer());
    TEST_ASSERT(!feq(center.x, 0.0));
    TEST_ASSERT(!feq(center.y, 0.0));
  }

  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

void testCoordgenMinimize() {
  BOOST_LOG(rdInfoLog) << "-------------------------------------" << std::endl;
  BOOST_LOG(rdInfoLog) << "testing coordgen minimize" << std::endl;
  {
    auto m1 =
        R"CTAB(
  Mrv2014 07302005442D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 10 11 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 3.3741 -12.4894 0 0
M  V30 2 C 4.7698 -13.1402 0 0
M  V30 3 C 6.0313 -12.2569 0 0
M  V30 4 C 5.8971 -10.7228 0 0
M  V30 5 C 4.5014 -10.072 0 0
M  V30 6 C 3.2399 -10.9553 0 0
M  V30 7 C 4.4148 -11.2907 0 0
M  V30 8 C 1.8442 -10.3045 0 0
M  V30 9 C 3.1057 -9.4212 0 0
M  V30 10 C 1.5715 -9.5554 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 5 1 5 6
M  V30 6 1 1 6
M  V30 7 1 1 7
M  V30 8 1 7 4
M  V30 9 1 6 8
M  V30 10 1 8 9
M  V30 11 1 9 10
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB"_ctab;
    TEST_ASSERT(m1);
    TEST_ASSERT(m1->getNumConformers() == 1);
    ROMol m2(*m1);
    // std::cerr << " FULL CG" << std::endl;
    // TEST_ASSERT(CoordGen::addCoords(m2) == 0);
    // std::cerr << MolToV3KMolBlock(m2) << std::endl;
    CoordGen::CoordGenParams ps;
    ps.minimizeOnly = true;
    std::cerr << " minimize only" << std::endl;
    CoordGen::addCoords(*m1, &ps);
    std::cerr << MolToV3KMolBlock(*m1) << std::endl;
  }
  {
    auto m1 =
        R"CTAB(
  Mrv2014 08042019502D          

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 19 21 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -6.3333 -4.7717 0 0
M  V30 2 C -7.667 -4.0017 0 0
M  V30 3 C -9.0008 -4.7717 0 0
M  V30 4 C -9.0008 -6.3117 0 0
M  V30 5 C -7.667 -7.0817 0 0
M  V30 6 C -6.3333 -6.3117 0 0
M  V30 7 C -4.9997 -7.0817 0 0
M  V30 8 C -3.6659 -6.3117 0 0
M  V30 9 C -3.6659 -4.7717 0 0
M  V30 10 C -4.9997 -4.0017 0 0
M  V30 11 C -2.3322 -4.0016 0 0
M  V30 12 C -2.5332 -5.5285 0 0
M  V30 13 C -1.4443 -4.4395 0 0
M  V30 14 C -9.0007 -3.2317 0 0
M  V30 15 C -7.9117 -2.1427 0 0
M  V30 16 O -10.0896 -4.3206 0 0
M  V30 17 C -7.9117 -0.6027 0 0
M  V30 18 C -6.6217 -1.0194 0 0
M  V30 19 C -6.3717 -2.1427 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 2 2 3
M  V30 3 1 3 4
M  V30 4 2 4 5
M  V30 5 1 5 6
M  V30 6 2 7 8
M  V30 7 1 8 9
M  V30 8 2 9 10
M  V30 9 2 1 6
M  V30 10 1 1 10
M  V30 11 1 6 7
M  V30 12 1 9 11
M  V30 13 1 11 12
M  V30 14 1 12 13
M  V30 15 1 2 14
M  V30 16 1 14 15
M  V30 17 2 14 16
M  V30 18 1 17 18
M  V30 19 1 18 19
M  V30 20 1 17 15
M  V30 21 1 15 19
M  V30 END BOND
M  V30 END CTAB
M  END
)CTAB"_ctab;
    TEST_ASSERT(m1);
    TEST_ASSERT(m1->getNumConformers() == 1);
    ROMol m2(*m1);
    // std::cerr << " FULL CG" << std::endl;
    // TEST_ASSERT(CoordGen::addCoords(m2) == 0);
    // std::cerr << MolToV3KMolBlock(m2) << std::endl;
    CoordGen::CoordGenParams ps;
    ps.minimizeOnly = true;
    std::cerr << " minimize only" << std::endl;
    CoordGen::addCoords(*m1, &ps);
    std::cerr << MolToV3KMolBlock(*m1) << std::endl;
  }
  BOOST_LOG(rdInfoLog) << "done" << std::endl;
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  RDLog::InitLogs();
#if 0
  test2();
  test1();
  testGithub1929();
  testGithub3131();
#endif
  testCoordgenMinimize();
}
