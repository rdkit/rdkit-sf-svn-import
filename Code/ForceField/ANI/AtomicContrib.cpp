//
//  Copyright (C) 2020 Manan Goel
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#include "AtomicContrib.h"
#include <ForceField/ForceField.h>
#include <RDGeneral/Invariant.h>
#include <RDGeneral/utils.h>
#include <Numerics/EigenSerializer/EigenSerializer.h>
#include <Eigen/Dense>
#include <GraphMol/Descriptors/AtomicEnvironmentVector.h>
#include <fstream>
using namespace Eigen;

namespace ForceFields {
namespace ANI {
ANIAtomContrib::ANIAtomContrib(ForceField *owner, int atomType,
                               unsigned int atomIdx, VectorXi &speciesVec,
                               unsigned int numAtoms, unsigned int numLayers,
                               unsigned int ensembleSize,
                               std::string modelType) {
  PRECONDITION(owner, "Bad Owner")
  PRECONDITION(atomType == 0 || atomType == 1 || atomType == 2 || atomType == 3,
               "Atom Type not Supported");
  PRECONDITION(modelType == "ANI-1x" || modelType == "ANI-1ccx",
               "Model Not currently supported")
  PRECONDITION(ensembleSize > 0,
               "There must be at least 1 model in the ensemble");
  URANGE_CHECK(atomIdx, numAtoms);
  dp_forceField = owner;
  this->d_atomType = atomType;
  this->d_atomIdx = atomIdx;
  this->d_speciesVec = speciesVec;
  this->d_numAtoms = numAtoms;
  this->d_ensembleSize = ensembleSize;
  this->d_modelType = modelType;

  if (this->d_atomEncoding.find(this->d_atomType) !=
      this->d_atomEncoding.end()) {
    auto atomicSymbol = this->d_atomEncoding[this->d_atomType];
    for (unsigned int modelNum = 0; modelNum < ensembleSize; modelNum++) {
      std::vector<ArrayXXd> currModelWeights;
      std::vector<ArrayXXd> currModelBiases;
      Utils::loadFromBin(&currModelWeights, &currModelBiases, modelNum,
                         atomicSymbol, this->d_modelType);
      this->d_weights.push_back(currModelWeights);
      this->d_biases.push_back(currModelBiases);
    }
    Utils::loadSelfEnergy(&(this->d_selfEnergy), atomicSymbol,
                          this->d_modelType);
  } else {
    this->d_selfEnergy = 0;
  }
}

double ANIAtomContrib::forwardProp(ArrayXXd &aev) const {
  if (this->d_atomType == -1) {
    return 0;
  }
  if (aev.cols() != 1) {
    aev.transposeInPlace();
  }

  std::vector<double> energies;
  energies.reserve(this->d_weights.size());
  for (unsigned int modelNo = 0; modelNo < this->d_weights.size(); modelNo++) {
    auto temp = aev;
    for (unsigned int layer = 0; layer < this->d_weights[modelNo].size();
         layer++) {
      temp =
          ((this->d_weights[modelNo][layer].matrix() * temp.matrix()).array() +
           this->d_biases[modelNo][layer])
              .eval();
      if (layer < this->d_weights[modelNo].size() - 1) Utils::CELU(temp, 0.1);
    }
    energies.push_back(temp.coeff(0, 0));
  }
  return std::accumulate(energies.begin(), energies.end(), 0.0) /
         energies.size();
}

double ANIAtomContrib::getEnergy(double *pos) const {
  auto aev = RDKit::Descriptors::ANI::AtomicEnvironmentVector(
      pos, this->d_speciesVec, this->d_numAtoms);
  ArrayXXd row = aev.row(this->d_atomIdx);
  return this->ANIAtomContrib::forwardProp(row) +
         this->d_selfEnergy;
}

double ANIAtomContrib::getEnergy(Eigen::ArrayXXd &aev) const {
  ArrayXXd row = aev.row(this->d_atomIdx);
  return this->ANIAtomContrib::forwardProp(row) +
         this->d_selfEnergy;
}

void ANIAtomContrib::getGrad(double *pos, double *grad) const {}

namespace Utils {

double RELU(double val) { return std::max(0.0, val); }

double coeffMin(double val) { return std::min(0.0, val); }

void CELU(ArrayXXd &input, double alpha) {
  input = input.unaryExpr(&RELU) +
          (alpha * ((input / alpha).exp() - 1)).unaryExpr(&coeffMin);
}

std::vector<std::string> tokenize(const std::string &s) {
  boost::char_separator<char> sep(", \n\r\t");
  boost::tokenizer<boost::char_separator<char>> tok(s, sep);
  std::vector<std::string> tokens;
  std::copy(tok.begin(), tok.end(),
            std::back_inserter<std::vector<std::string>>(tokens));
  return tokens;
}

void loadFromBin(std::vector<ArrayXXd> *weights, unsigned int model,
                 std::string weightType, unsigned int layer,
                 std::string atomType, std::string modelType) {
  std::string path = getenv("RDBASE");
  std::string paramFile = path + "/Code/ForceField/ANI/Params/" + modelType +
                          "/model" + std::to_string(model) + "/" + atomType +
                          "_" + std::to_string(layer) + "_" + weightType +
                          ".bin";
  ArrayXXf weight;
  RDNumeric::EigenSerializer::deserialize(weight, paramFile);
  weights->push_back(weight.matrix().cast<double>().array());
}

void loadFromBin(std::vector<ArrayXXd> *weights, std::vector<ArrayXXd> *biases,
                 unsigned int model, std::string atomType,
                 std::string modelType) {
  std::string path = getenv("RDBASE");
  std::string paramFile = path + "/Code/ForceField/ANI/Params/" + modelType +
                          "/model" + std::to_string(model) + ".bin";
  std::vector<ArrayXXf> floatWeights, floatBiases;
  RDNumeric::EigenSerializer::deserializeAll(&floatWeights, &floatBiases,
                                             paramFile, atomType);
  for (unsigned int i = 0; i < floatWeights.size(); i++) {
    weights->push_back(floatWeights[i].matrix().cast<double>().array());
    biases->push_back(floatBiases[i].matrix().cast<double>().array());
  }
}

void loadFromCSV(std::vector<ArrayXXd> *weights, unsigned int model,
                 std::string weightType, unsigned int layer,
                 std::string atomType, std::string modelType) {
  std::string path = getenv("RDBASE");
  std::string paramFile = path + "/Code/ForceField/ANI/Params/" + modelType +
                          "/model" + std::to_string(model) + "/" + atomType +
                          "_" + std::to_string(layer) + "_" + weightType;

  std::ifstream instrmSF(paramFile.c_str());
  if (!instrmSF.good()) {
    throw ValueErrorException(paramFile + " Model File does not exist");
    return;
  }
  std::string line;
  std::vector<std::string> tokens;
  std::vector<std::vector<double>> weight;
  unsigned int cols = 1;
  while (!instrmSF.eof()) {
    std::getline(instrmSF, line);
    tokens = tokenize(line);
    std::vector<double> row;
    for (auto v : tokens) {
      std::istringstream os(v);
      double d;
      os >> d;
      row.push_back(d);
    }
    if (row.size() > 0) {
      cols = row.size();
      weight.push_back(row);
    }
  }

  ArrayXXd param(weight.size(), cols);

  for (unsigned int i = 0; i < weight.size(); i++) {
    for (unsigned int j = 0; j < weight[i].size(); j++) {
      param(i, j) = weight[i][j];
    }
  }
  weights->push_back(param);
}

void loadSelfEnergy(double *energy, std::string atomType,
                    std::string modelType) {
  std::string path = getenv("RDBASE");
  std::string filePath =
      path + "/Code/ForceField/ANI/Params/" + modelType + "/selfEnergies";

  std::ifstream selfEnergyFile(filePath.c_str());
  if (!selfEnergyFile.good()) {
    throw ValueErrorException(filePath + " : File Does Not Exist");
    return;
  }
  std::string line;
  while (!selfEnergyFile.eof()) {
    std::getline(selfEnergyFile, line);
    boost::char_separator<char> sep(" ,=");
    boost::tokenizer<boost::char_separator<char>> tok(line, sep);
    std::vector<std::string> tokens;
    std::copy(tok.begin(), tok.end(),
              std::back_inserter<std::vector<std::string>>(tokens));

    if (tokens[0] == atomType) {
      std::istringstream os(tokens[2]);
      os >> *energy;
      break;
    }
  }
  selfEnergyFile.close();
}

}  // namespace Utils
}  // namespace ANI
}  // namespace ForceFields