//
//  Copyright (C) 2020 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <RDGeneral/export.h>
#ifndef RD_PNGPARSER_H
#define RD_PNGPARSER_H

#include <RDGeneral/types.h>
#include <RDGeneral/BadFileException.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>

#include <boost/format.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <map>

namespace RDKit {

namespace PNGData {
RDKIT_FILEPARSERS_EXPORT extern const std::string smilesTag;
RDKIT_FILEPARSERS_EXPORT extern const std::string molTag;
}  // namespace PNGData

RDKIT_FILEPARSERS_EXPORT std::map<std::string, std::string> PNGStreamToMetadata(
    std::istream &inStream);
inline std::map<std::string, std::string> PNGFileToMetadata(
    const std::string &fname) {
  std::ifstream inStream(fname.c_str(), std::ios::binary);
  if (!inStream || (inStream.bad())) {
    throw BadFileException((boost::format("Bad input file %s") % fname).str());
  }
  return PNGStreamToMetadata(inStream);
}
inline std::map<std::string, std::string> PNGStringToMetadata(
    const std::string &data) {
  std::stringstream inStream(data);
  return PNGStreamToMetadata(inStream);
}

RDKIT_FILEPARSERS_EXPORT ROMol *PNGStreamToMol(
    std::istream &inStream,
    const SmilesParserParams &params = SmilesParserParams());
inline ROMol *PNGFileToMol(
    const std::string &fname,
    const SmilesParserParams &params = SmilesParserParams()) {
  std::ifstream inStream(fname.c_str(), std::ios::binary);
  if (!inStream || (inStream.bad())) {
    throw BadFileException((boost::format("Bad input file %s") % fname).str());
  }
  return PNGStreamToMol(inStream, params);
}
inline ROMol *PNGStringToMol(
    const std::string &data,
    const SmilesParserParams &params = SmilesParserParams()) {
  std::stringstream inStream(data);
  return PNGStreamToMol(inStream, params);
}

RDKIT_FILEPARSERS_EXPORT std::string addMetadataToPNGStream(
    std::istream &iStream, const std::map<std::string, std::string> &metadata);
inline std::string addMetadataToPNGString(
    const std::string &pngString,
    const std::map<std::string, std::string> &metadata) {
  std::stringstream inStream(pngString);
  return addMetadataToPNGStream(inStream, metadata);
}
inline std::string addMetadataToPNGFile(
    const std::string &fname,
    const std::map<std::string, std::string> &metadata) {
  std::ifstream inStream(fname.c_str(), std::ios::binary);
  return addMetadataToPNGStream(inStream, metadata);
}

RDKIT_FILEPARSERS_EXPORT std::string addMolToPNGStream(std::istream &iStream,
                                                       const ROMol &mol);
inline std::string addMolToPNGString(
    const std::string &pngString, const ROMol &mol) {
  std::stringstream inStream(pngString);
  return addMolToPNGStream(inStream, mol);
}
inline std::string addMolToPNGFile(const std::string &fname,
                                                     const ROMol &mol) {
  std::ifstream inStream(fname.c_str(), std::ios::binary);
  return addMolToPNGStream(inStream, mol);
}

}  // namespace RDKit

#endif
