//
//  Copyright (C) 2019 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#define NO_IMPORT_ARRAY
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <string>

// ours
#include <GraphMol/RDKitBase.h>
#include <GraphMol/Sgroup.h>
#include "props.hpp"

namespace python = boost::python;

namespace RDKit {

namespace {
python::tuple getMolSGroups(ROMol &mol) {
  python::list res;
  std::vector<SGroup> &sgs = getSGroups(mol);
  for (auto &sg : sgs) {
    res.append(&sg);
  }
  return python::tuple(res);
}
void clearMolSGroups(ROMol &mol) {
  std::vector<SGroup> &sgs = getSGroups(mol);
  sgs.clear();
}
}  // namespace

std::string sGroupClassDoc =
    "A collection of atoms and bonds with associated properties\n";

struct sgroup_wrap {
  static void wrap() {
    python::class_<SGroup, boost::shared_ptr<SGroup>>(
        "SGroup", sGroupClassDoc.c_str(), python::no_init)
        .def("GetOwningMol", &SGroup::getOwningMol,
             "returns the molecule owning this SGroup",
             python::return_internal_reference<>())
        .def("GetIndexInMol", &SGroup::getIndexInMol,
             "returns the index of this SGroup in the owning molecule's list.")
        .def("GetAtoms", &SGroup::getAtoms,
             "returns a list of the indices of the atoms in this SGroup",
             python::return_value_policy<python::copy_const_reference>())
        .def("GetParentAtoms", &SGroup::getParentAtoms,
             "returns a list of the indices of the parent atoms in this SGroup",
             python::return_value_policy<python::copy_const_reference>())
        .def("GetBonds", &SGroup::getBonds,
             "returns a list of the indices of the bonds in this SGroup",
             python::return_value_policy<python::copy_const_reference>())
        .def("HasProp",
             (bool (RDProps::*)(const std::string &) const) & SGroup::hasProp,
             "returns whether or not a particular property exists")
        .def("GetProp",
             (std::string(RDProps::*)(const std::string &) const) &
                 SGroup::getProp<std::string>,
             "returns the value of a particular property")
        .def("GetIntProp",
             (int (RDProps::*)(const std::string &) const) &
                 SGroup::getProp<int>,
             "returns the value of a particular property")
        .def("GetUnsignedProp",
             (unsigned int (RDProps::*)(const std::string &) const) &
                 SGroup::getProp<unsigned int>,
             "returns the value of a particular property")
        .def("GetDoubleProp",
             (double (RDProps::*)(const std::string &) const) &
                 SGroup::getProp<double>,
             "returns the value of a particular property")
        .def("GetBoolProp",
             (bool (RDProps::*)(const std::string &) const) &
                 SGroup::getProp<bool>,
             "returns the value of a particular property")
        .def("GetPropNames", &SGroup::getPropList,
             (python::arg("self"), python::arg("includePrivate") = false,
              python::arg("includeComputed") = false),
             "Returns a list of the properties set on the SGroup.\n\n")
        .def("GetPropsAsDict", GetPropsAsDict<SGroup>,
             (python::arg("self"), python::arg("includePrivate") = true,
              python::arg("includeComputed") = true),
             "Returns a dictionary of the properties set on the SGroup.\n"
             " n.b. some properties cannot be converted to python types.\n");
    python::def("GetMolSGroups", &getMolSGroups,
                "returns the SGroups for a molecule (if any)");
    python::def("ClearMolSGroups", &clearMolSGroups,
                "removes all SGroups from a molecule (if any)");
    // FIX: needs something tying the lifetime to the mol
  }
};
}  // namespace RDKit

void wrap_sgroup() { RDKit::sgroup_wrap::wrap(); }
