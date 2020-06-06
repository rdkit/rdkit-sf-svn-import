//
// Created by Gareth Jones on 5/30/2020.
//
// Copyright 2020 Schrodinger, Inc
//

#include <RDBoost/Wrap.h>
#include <RDBoost/python.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <GraphMol/TautomerQuery/TautomerQuery.h>
#include <GraphMol/Wrap/substructmethods.h>

#define PY_ARRAY_UNIQUE_SYMBOL rdtautomerquery_array_API

namespace python = boost::python;
using namespace RDKit;

namespace {

TautomerQuery *createDefaultTautomerQuery(const ROMol &mol) {
  return TautomerQuery::fromMol(mol);
}

bool tautomerIsSubstructOf(const TautomerQuery &self, const ROMol &target,
                           bool recursionPossible = true,
                           bool useChirality = false,
                           bool useQueryQueryMatches = false) {
  return HasSubstructMatch(target, self, recursionPossible, useChirality,
                           useQueryQueryMatches);
}

PyObject *tautomerGetSubstructMatch(const TautomerQuery &self,
                                    const ROMol &target,
                                    bool useChirality = false,
                                    bool useQueryQueryMatches = false) {
  return GetSubstructMatch(target, self, useChirality, useQueryQueryMatches);
}

PyObject *tautomerGetSubstructMatches(const TautomerQuery &self,
                                      const ROMol &target, bool uniquify = true,
                                      bool useChirality = false,
                                      bool useQueryQueryMatches = false,
                                      unsigned int maxMatches = 1000) {
  return GetSubstructMatches(target, self, uniquify, useChirality,
                             useQueryQueryMatches, maxMatches);
}

PyObject *tautomerGetSubstructMatchesWithTautomers(
    const TautomerQuery &self, const ROMol &target, bool uniquify = true,
    bool useChirality = false, bool useQueryQueryMatches = false,
    unsigned int maxMatches = 1000) {
  std::vector<MatchVectType> matches;
  std::vector<ROMOL_SPTR> matchingTautomers;
  SubstructMatchParameters params;
  params.uniquify = uniquify;
  params.useChirality = useChirality;
  params.useQueryQueryMatches = useQueryQueryMatches;
  params.maxMatches = maxMatches;

  {
    NOGIL gil;

    SubstructMatchParameters matchParamters;
    matches = self.substructOf(target, params, &matchingTautomers);
  }
  int const numberMatches = matches.size();
  PyObject *res = PyTuple_New(numberMatches);
  for (int idx = 0; idx < numberMatches; idx++) {
    PyObject *pair = PyTuple_New(2);
    PyTuple_SetItem(pair, 0, convertMatches(matches[idx]));
    PyTuple_SetItem(
        pair, 1,
        python::converter::shared_ptr_to_python(matchingTautomers[idx]));
    PyTuple_SetItem(res, idx, pair);
  }
  return res;
}

} // namespace

struct TautomerQuery_wrapper {
  static void wrap() {
    RegisterVectorConverter<size_t>("UnsignedLong_Vect");
    RegisterVectorConverter<ROMOL_SPTR>("Mol_Vect");
    auto docString =
        "The Tautomer Query Class.\n\
  Creates a query that enables structure search accounting for matching of\n\
  Tautomeric forms\n";

    python::class_<TautomerQuery, boost::noncopyable>(
        "TautomerQuery", docString, python::no_init)
        .def("__init__", python::make_constructor(createDefaultTautomerQuery))
        .def("__init__", python::make_constructor(&TautomerQuery::fromMol))
        .def("IsSubstructOf", tautomerIsSubstructOf,
             (python::arg("self"), python::arg("target"),
              python::arg("recursionPossible") = true,
              python::arg("useChirality") = false,
              python::arg("useQueryQueryMatches") = false))
        .def("GetSubstructMatch", tautomerGetSubstructMatch,
             (python::arg("self"), python::arg("target"),
              python::arg("useChirality") = false,
              python::arg("useQueryQueryMatches") = false))
        .def("GetSubstructMatches", tautomerGetSubstructMatches,
             (python::arg("self"), python::arg("target"),
              python::arg("uniquify") = true,
              python::arg("useChirality") = false,
              python::arg("useQueryQueryMatches") = false,
              python::arg("maxMatches") = 1000))
        .def("GetSubstructMatchesWithTautomers",
             tautomerGetSubstructMatchesWithTautomers,
             (python::arg("self"), python::arg("target"),
              python::arg("uniquify") = true,
              python::arg("useChirality") = false,
              python::arg("useQueryQueryMatches") = false,
              python::arg("maxMatches") = 1000))
        .def("PatternFingerprintTemplate",
             &TautomerQuery::patternFingerprintTemplate,
             (python::arg("fingerprintSize") = 2048),
             python::return_value_policy<python::manage_new_object>())
        .def("GetTemplateMolecule", &TautomerQuery::getTemplateMolecule,
             python::return_internal_reference<>())
        .def("GetModifiedAtoms", &TautomerQuery::getModifiedAtoms)
        .def("GetModifiedBonds", &TautomerQuery::getModifiedBonds)
        .def("GetTautomers", &TautomerQuery::getTautomers);

    python::def("PatternFingerprintTautomerTarget",
                &TautomerQuery::patternFingerprintTarget,
                (python::arg("target"), python::arg("fingerprintSize") = 2048),
                python::return_value_policy<python::manage_new_object>());
  };
};


void wrap_TautomerQuery() { TautomerQuery_wrapper::wrap(); }

BOOST_PYTHON_MODULE(rdTautomerQuery) { wrap_TautomerQuery(); }
