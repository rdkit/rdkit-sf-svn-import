//
//  Copyright (C) 2020 Gareth Jones, Glysade LLC
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#include <ctime>
#include <limits>
#include <future>
#include "RGroupGa.h"
#include "RGroupDecompData.h"
#include "RGroupDecomp.h"
#include "../../../External/GA/util/Util.h"

namespace RDKit {

using namespace std;

RGroupDecompositionChromosome::RGroupDecompositionChromosome(RGroupGa& rGroupGa)
    : IntegerStringChromosome(rGroupGa.chromosomeLength(), rGroupGa.getRng(),
                              rGroupGa.getChromosomePolicy()),
      rGroupGa(rGroupGa) {
  permutation.reserve(rGroupGa.numberDecompositions());
}

std::string RGroupDecompositionChromosome::info() const {
  auto format = boost::format("Fit %7.3f : ") % fitness;
  return format.str() + geneInfo();
}

double RGroupDecompositionChromosome::score() {
  auto& rGroupData = rGroupGa.getRGroupData();
  RGroupScore scoreMethod =
      static_cast<RGroupScore>(rGroupData.params.scoreMethod);
  if (operationName != RgroupMutate) {
    decode();
  }
  if (scoreMethod == FingerprintVariance && labelsToVarianceData.size() > 0 &&
      operationName == RgroupMutate) {
    fitness = fingerprintVarianceGroupScore(labelsToVarianceData);
    // assert(fitness == recalculateScore());
  } else {
    fitness = rGroupData.score(permutation, &labelsToVarianceData);
  }
  return fitness;
}

double RGroupDecompositionChromosome::recalculateScore() {
  std::cerr << "Recalculating score" << std::endl;
  auto& rGroupData = rGroupGa.getRGroupData();
  return rGroupData.score(permutation);
}

void RGroupDecompositionChromosome::decode() {
  auto values = getString();
  permutation.clear();
  auto& matches = rGroupGa.getRGroupData().matches;
  auto pos = 0;
  for (const auto& m : matches) {
    if (m.size() == 1) {
      permutation.push_back(0);
    } else {
      permutation.push_back(values[pos]);
      pos++;
    }
  }
}

void RGroupDecompositionChromosome::copyGene(
    const StringChromosomeBase<int, IntegerStringChromosomePolicy>& other) {
  StringChromosomeBase<int, IntegerStringChromosomePolicy>::copyGene(other);
  const auto& parent = static_cast<const RGroupDecompositionChromosome&>(other);
  copyVarianceData(parent.labelsToVarianceData, labelsToVarianceData);
}

GaResult& GaResult::operator=(const GaResult& other) {
  if (&other == this) {
    return *this;
  }
  score = other.score;
  permutations = other.permutations;
  return *this;
}

RGroupGa::RGroupGa(const RGroupDecompData& rGroupData,
                   const chrono::steady_clock::time_point* const t0)
    : rGroupData(rGroupData),
      chromosomePolicy(getRng(), rGroupData.matches.size()),
      t0(t0) {
  setSelectionPressure(1.0001);

  const auto& matches = rGroupData.matches;
  numPermutations = 0L;
  auto pos = 0;
  for (auto m : matches) {
    if (m.size() == 1) continue;
    chromosomePolicy.setMax(pos, m.size());
    unsigned long count = numPermutations = m.size();
    numPermutations = count / m.size() == numPermutations
                          ? count
                          : numeric_limits<unsigned int>::max();
    pos++;
  }
  chromLength = pos;
  numberDecomps = matches.size();

  // TODO refine these settings
  auto popsize = 100 + chromLength / 10;
  if (popsize > 200) popsize = 200;
  const auto& params = rGroupData.params;
  if (params.gaPopulationSize > 0) {
    popsize = params.gaPopulationSize;
  }
  // numberOperations = 5000 + chromLength * 100;
  // if (numberOperations > 100000) numberOperations = 100000;

  // For now run the GA a long time and exit early if no improvement in the
  // score is seen
  numberOperations = 1000000;
  if (params.gaMaximumOperations > 0) {
    numberOperations = params.gaMaximumOperations;
  }

  numberOperationsWithoutImprovement = 7500;
  if (params.gaNumberOperationsWithoutImprovement > 0) {
    numberOperationsWithoutImprovement =
        params.gaNumberOperationsWithoutImprovement;
  }

  // profiler settings
  // popsize = 100;
  // numberOperations = 10000;

  setPopsize(popsize);

  uint32_t rngSeed;

  if (params.gaRandomSeed >= 0) {
    rngSeed = params.gaRandomSeed;
    getRng().seed(rngSeed);
  } else if (params.gaRandomSeed == -2) {
    random_device rd;
    auto seed = rd();
    rngSeed = seed;
    getRng().seed(rngSeed);
  } else {
    rngSeed = mt19937::default_seed;
  }

  BOOST_LOG(rdInfoLog) << "GA RNG seed " << rngSeed << endl;
}

void RGroupGa::rGroupMutateOperation(
    const std::vector<std::shared_ptr<RGroupDecompositionChromosome>>& parents,
    std::vector<std::shared_ptr<RGroupDecompositionChromosome>>& children) {
  auto parent = parents[0];
  auto child = children[0];
  child->copyGene(*parent);
  child->mutate();
  child->setOperationName(RgroupMutate);
  child->decode();

  auto& labelsToVarianceData = child->getLabelsToVarianceData();
  if (labelsToVarianceData.size() == 0) return;
#ifdef DEBUG
  std::cerr << "RGroup mutate start" << std::endl;
#endif
#ifdef DEBUG
  std::cerr << "Starting child score" << std::endl;
  fingerprintVarianceScore(labelsToVarianceData);
#endif

  auto& parentPermutation = parent->getPermutation();
  auto& childPermutation = child->getPermutation();
  const auto& rgroupData = parent->getRGroupGA().getRGroupData();
  const auto& matches = rgroupData.matches;
  const auto& labels = rgroupData.labels;

  for (auto pos = 0U; pos < parentPermutation.size(); pos++) {
    int parentValue = parentPermutation.at(pos);
    int childValue = childPermutation.at(pos);
    if (parentValue != childValue) {
      removeVarianceData(pos, parentValue, matches, labels,
                         labelsToVarianceData);
#ifdef DEBUG
      std::cerr << "After removing parent" << std::endl;
      fingerprintVarianceScore(labelsToVarianceData);
#endif
      addVarianceData(pos, childValue, matches, labels, labelsToVarianceData);
#ifdef DEBUG
      std::cerr << "After adding child" << std::endl;
      fingerprintVarianceScore(labelsToVarianceData);
#endif
    }
  }
#ifdef DEBUG
  std::cerr << "Final recalculating" << std::endl;
  child->recalculateScore();
  std::cerr << "RGroup mutate done" << std::endl;
#endif
}

void RGroupGa::rGroupCrossoverOperation(
    const std::vector<std::shared_ptr<RGroupDecompositionChromosome>>& parents,
    std::vector<std::shared_ptr<RGroupDecompositionChromosome>>& children) {
  auto parent1 = parents[0];
  auto child1 = children[0];
  auto parent2 = parents[1];
  auto child2 = children[1];

  child1->setOperationName(Crossover);
  child2->setOperationName(Crossover);
  clearVarianceData(child1->getLabelsToVarianceData());
  clearVarianceData(child2->getLabelsToVarianceData());

  parent1->twoPointCrossover(*parent2, *child1, *child2);
}

const vector<shared_ptr<GaOperation<RGroupDecompositionChromosome>>>
RGroupGa::getOperations() const {
  // bias to mutation as that operator is so efficient
  auto mutationOperation =
      make_shared<GaOperation<RGroupDecompositionChromosome>>(
          1, 1, 75.0, &rGroupMutateOperation);
  auto crossoverOperation =
      make_shared<GaOperation<RGroupDecompositionChromosome>>(
          2, 2, 25.0, &rGroupCrossoverOperation);
  vector<shared_ptr<GaOperation<RGroupDecompositionChromosome>>> operations;
  operations.reserve(2);
  operations.push_back(mutationOperation);
  operations.push_back(crossoverOperation);
  return operations;
}

std::string timeInfo(const std::clock_t start) {
  auto now = std::clock();
  auto seconds = (now - start) / (double)CLOCKS_PER_SEC;
  auto format = boost::format("Time %7.2f") % seconds;
  return format.str();
}

GaResult RGroupGa::run(int runNumber) {
  auto startTime = clock();
  RGroupGaPopulation population{*this};
  auto format =
      boost::format(
          "Running GA run %2d number operations %5d population size %5d "
          "number operations without improvement %5d "
          "chromosome length %5d %s\n") %
      runNumber % numberOperations % getPopsize() %
      numberOperationsWithoutImprovement % chromLength % timeInfo(startTime);
  BOOST_LOG(rdInfoLog) << format.str();
  population.create();
  double bestScore = population.getBestScore();
  BOOST_LOG(rdInfoLog) << population.info() << endl;
  BOOST_LOG(rdDebugLog) << population.populationInfo();

  int nOps = 0;
  int lastImprovementOp = 0;
  while (nOps < numberOperations) {
    population.iterate();
    nOps++;
    if (nOps % 1000 == 0) {
      BOOST_LOG(rdInfoLog) << "Run " << runNumber << " " << population.info() << " "
                           << timeInfo(startTime) << endl;
    }
    if (population.getBestScore() > bestScore) {
      bestScore = population.getBestScore();
      lastImprovementOp = nOps;
      auto format = boost::format("Run %2d OP %5d Fit %7.3f %s\n") % runNumber %
                    nOps % bestScore % timeInfo(startTime);
      BOOST_LOG(rdInfoLog) << format.str();
    }
    if (nOps - lastImprovementOp > numberOperationsWithoutImprovement) {
      BOOST_LOG(rdInfoLog) << "Run " << runNumber << " Op " << nOps
                           << " No improvement since " << lastImprovementOp
                           << " finishing.." << endl;
      break;
    }
    if (t0 && checkForTimeout(*t0, rGroupData.params.timeout)) {
      break;
    }
  }
  const shared_ptr<RGroupDecompositionChromosome> best = population.getBest();
  BOOST_LOG(rdDebugLog) << "Run " << runNumber << " Best solution " << best->info() << endl;
  BOOST_LOG(rdDebugLog) << "Run " << runNumber << " " << population.populationInfo();

  auto ties = population.getTiedBest();
  vector<vector<size_t>> permutations;
  permutations.reserve(ties.size());
  std::transform(ties.cbegin(), ties.cend(), back_inserter(permutations),
                 [](const shared_ptr<RGroupDecompositionChromosome> c) {
                   return c->getPermutation();
                 });
  BOOST_LOG(rdInfoLog) << "Run " << runNumber << " Execution " << timeInfo(startTime) << std::endl;
  GaResult result{best->getFitness(), permutations};
  return result;
}

vector<GaResult> RGroupGa::runBatch() {
  int numberRuns = rGroupData.params.gaNumberRuns;
  vector<GaResult> results;
  results.reserve(numberRuns);

  if (rGroupData.params.gaParallelRuns) {
    vector<future<GaResult>> tasks;
    tasks.reserve(numberRuns);
    for (int n = 0; n < numberRuns; n++) {
      auto future = async(launch::async, &RDKit::RGroupGa::run, this, n + 1);
      tasks.push_back(move(future));
    }

    std::transform(tasks.begin(), tasks.end(), back_inserter(results),
                   [](future<GaResult>& f) { return f.get(); });
  } else {
    for (int n = 0; n < numberRuns; n++) {
      auto result = run(n + 1);
      results.push_back(result);
    }
  }

  return results;
}

shared_ptr<RGroupDecompositionChromosome> RGroupGa::createChromosome() {
  return make_shared<RGroupDecompositionChromosome>(*this);
}

void copyVarianceData(const map<int, shared_ptr<VarianceDataForLabel>>& from,
                      map<int, shared_ptr<VarianceDataForLabel>>& to) {
  for (auto it = from.cbegin(); it != from.end(); ++it) {
    auto df = to.find(it->first);
    if (df == to.end()) {
      to.emplace(it->first, make_shared<VarianceDataForLabel>(*it->second));
    } else {
      auto& fromData = it->second;
      auto& toData = df->second;
      toData->numberFingerprints = fromData->numberFingerprints;
      toData->bitCounts.assign(fromData->bitCounts.cbegin(),
                               fromData->bitCounts.cend());
    }
  }
}

void clearVarianceData(map<int, shared_ptr<VarianceDataForLabel>>& data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    auto d = it->second;
    d->numberFingerprints = 0;
    d->bitCounts.assign(d->bitCounts.size(), 0.0);
  }
}

}  // namespace RDKit