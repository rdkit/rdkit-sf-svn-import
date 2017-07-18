//  Copyright (c) 2017, Novartis Institutes for BioMedical Research Inc.
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Novartis Institutes for BioMedical Research Inc.
//       nor the names of its contributors may be used to endorse or promote
//       products derived from this software without specific prior written
//       permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include <RDBoost/python.h>
#include <RDBoost/Wrap.h>
#include <GraphMol/RDKitBase.h>

#include <GraphMol/SubstructLibrary/SubstructLibrary.h>

namespace python = boost::python;

namespace RDKit {

const char * MolHolderBaseDoc = "";
const char * MolHolderDoc = "";
const char * CachedMolHolderDoc = "";
const char * CachedSmilesMolHolderDoc = "";
const char * PatternHolderDoc = "";
const char * SubstructLibraryDoc = "";

struct substructlibrary_wrapper {
    static void wrap() {
      // n.b. there can only be one of these in all wrappings
      //python::class_<std::vector<unsigned int> >("UIntVect").def(
      //  python::vector_indexing_suite<std::vector<unsigned int>, true>());

      python::class_<MolHolderBase, boost::noncopyable>(
          "MolHolderBase", "", python::no_init)
          
          .def("AddMol", &MolHolderBase::addMol,
               "Adds molecle to the molecule holder")
          .def("GetMol", &MolHolderBase::getMol,
               "Returns a particular molecule in the molecule holder\n\n"
               "  ARGUMENTS:\n"
               "    - idx: which molecule to return\n\n"
               "  NOTE: molecule indices start at 0\n")
          .def("__len__", &MolHolderBase::size);

      python::class_<MolHolder, boost::shared_ptr<MolHolder>, python::bases<MolHolderBase> >(
          "MolHolder", MolHolderDoc, python::init<>());

      python::class_<CachedMolHolder, boost::shared_ptr<CachedMolHolder>, python::bases<MolHolderBase > >(
          "CachedMolHolder", CachedMolHolderDoc, python::init<>())
          .def("AddBinary", &CachedMolHolder::addBinary,
               (python::args("pickle")),
               "Add a binary pickle to the molecule holder, no checking is done on the input data");
          

      python::class_<CachedSmilesMolHolder, boost::shared_ptr<CachedSmilesMolHolder>, python::bases<MolHolderBase > >(
          "CachedSmilesMolHolder", CachedSmilesMolHolderDoc, python::init<>())
          .def("AddTrustedSmiles", &CachedSmilesMolHolder::addTrustedSmiles,
               (python::args("smiles")),
               "Add a trusted smiles string to the molecule holder, no checking is done on the input data");
               

      python::class_<FPHolderBase, boost::shared_ptr<FPHolderBase>, boost::noncopyable>(
          "FPHolderBase", "", python::no_init)
          .def("AddMol", &FPHolderBase::addMol,
               "Adds molecle to the fingerprint database")
          .def("PassesFilter", &FPHolderBase::passesFilter);
      
      python::class_<PatternHolder, boost::shared_ptr<PatternHolder>, python::bases<FPHolderBase> >(
          "PatternHolder", PatternHolderDoc, python::init<>());
      
      python::class_<SubstructLibrary, SubstructLibrary *,
                     const SubstructLibrary *>(
                         "SubstructLibrary", SubstructLibraryDoc, python::init<>())
          .def(python::init<boost::shared_ptr<MolHolderBase> >())          
          .def(python::init<boost::shared_ptr<MolHolderBase>, boost::shared_ptr<FPHolderBase> >())
          
          .def("AddMol",  &SubstructLibrary::addMol,
               (python::arg("mol")),
               "Adds a molecule to the substruct library")


          .def("GetMatches", (std::vector<unsigned int> (SubstructLibrary::*)(
              const ROMol &, bool, bool, bool, int, int)) &SubstructLibrary::getMatches,
               (python::arg("query"),
                python::arg("recursionPossible")=true,
                python::arg("useChirality")=true,
                python::arg("useQueryQueryMatches")=false,
                python::arg("numThreads")=-1,
                python::arg("maxResults")=1000),
               "Get the matches for the query.\n\n"
               " Arguments:\n"
               "  - query:      substructure query\n"
               "  - numThreads: number of threads to use, -1 means all threads\n"
               "  - maxResults: maximum number of results to return")


          .def("GetMatches", (std::vector<unsigned int> (SubstructLibrary::*)(
              const ROMol &, unsigned int, unsigned int, bool, bool, bool, 
              int, int)) &SubstructLibrary::getMatches,
               (python::arg("query"),
                python::arg("startIdx"),
                python::arg("endIdx"),
                python::arg("recursionPossible")=true,
                python::arg("useChirality")=true,
                python::arg("useQueryQueryMatches")=false,                
                python::arg("numThreads")=-1,
                python::arg("maxResults")=1000),
               "Get the matches for the query.\n\n"
               " Arguments:\n"
               "  - query:      substructure query\n"
               "  - startIdx:   index to search from\n"
               "  - endIdx:     index (non-inclusize) to search to\n"
               "  - numThreads: number of threads to use, -1 means all threads\n"
               "  - maxResults: maximum number of results to return")

          .def("CountMatches", (unsigned int (SubstructLibrary::*)(
              const ROMol &, bool, bool, bool, int)) &SubstructLibrary::countMatches,
               (python::arg("query"),
                python::arg("recursionPossible")=true,
                python::arg("useChirality")=true,
                python::arg("useQueryQueryMatches")=false,                
                python::arg("numThreads")=-1,
                python::arg("maxResults")=1000),
               "Get the matches for the query.\n\n"
               " Arguments:\n"
               "  - query:      substructure query\n"
               "  - numThreads: number of threads to use, -1 means all threads\n")


          .def("CountMatches", (unsigned int (SubstructLibrary::*)(
              const ROMol &, unsigned int, unsigned int,
              bool,bool,bool,
              int)) &SubstructLibrary::countMatches,
               (python::arg("query"),
                python::arg("startIdx"),
                python::arg("endIdx"),
                python::arg("recursionPossible")=true,
                python::arg("useChirality")=true,
                python::arg("useQueryQueryMatches")=false,                
                python::arg("numThreads")=-1),
               "Get the matches for the query.\n\n"
               " Arguments:\n"
               "  - query:      substructure query\n"
               "  - startIdx:   index to search from\n"
               "  - endIdx:     index (non-inclusize) to search to\n"
               "  - numThreads: number of threads to use, -1 means all threads\n")

          .def("HasMatch", (bool (SubstructLibrary::*)(
              const ROMol &, bool, bool, bool, int)) &SubstructLibrary::hasMatch,
               (python::arg("query"),
                python::arg("recursionPossible")=true,
                python::arg("useChirality")=true,
                python::arg("useQueryQueryMatches")=false,                
                python::arg("numThreads")=-1),
               "Get the matches for the query.\n\n"
               " Arguments:\n"
               "  - query:      substructure query\n"
               "  - numThreads: number of threads to use, -1 means all threads\n")

          .def("HasMatch", (bool (SubstructLibrary::*)(
              const ROMol &, unsigned int, unsigned int,
              bool, bool, bool, 
              int)) &SubstructLibrary::hasMatch,
               (python::arg("query"),
                python::arg("startIdx"),
                python::arg("endIdx"),
                python::arg("recursionPossible")=true,
                python::arg("useChirality")=true,
                python::arg("useQueryQueryMatches")=false,                
                python::arg("numThreads")=-1),
               "Get the matches for the query.\n\n"
               " Arguments:\n"
               "  - query:      substructure query\n"
               "  - startIdx:   index to search from\n"
               "  - endIdx:     index (non-inclusize) to search to\n"
               "  - numThreads: number of threads to use, -1 means all threads\n")
          

          .def("GetMol", &SubstructLibrary::getMol,
               "Returns a particular molecule in the molecule holder\n\n"
               "  ARGUMENTS:\n"
               "    - idx: which molecule to return\n\n"
               "  NOTE: molecule indices start at 0\n")

          .def("__len__", &SubstructLibrary::size)
          
          ;
    }
};
}

void wrap_substructlibrary() { RDKit::substructlibrary_wrapper::wrap(); }
