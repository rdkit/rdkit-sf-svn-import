//
// Created by gareth on 11/4/20.
//

#ifndef RDKIT_RGROUPFINGERPRINTSCORE_H
#define RDKIT_RGROUPFINGERPRINTSCORE_H

#include "RGroupMatch.h"
#include <vector>

namespace RDKit {

// class to hold the bitcounts for an attachment point/rgroup label
struct VarianceDataForLabel {
  // rgroup label
  const int label;
  // number of structures attached here
  int numberFingerprints;
  // bitcounts - size fingerprint size, each position is the count of bits set
  // over the fingerprints for all the structures
  std::vector<int> bitCounts;

  VarianceDataForLabel(const int &label, int numberFingerprints,
                       const std::vector<int> &bitCounts);
  VarianceDataForLabel(const int &label);
  VarianceDataForLabel(const VarianceDataForLabel &other) = default;
  VarianceDataForLabel &operator=(const VarianceDataForLabel &other) = delete;
  // add an rgroup structure to a bit counts array
  void addRgroupData(RGroupData *rgroupData);
  // remove an rgroup structure to a bit counts array
  void removeRgroupData(RGroupData *rgroupData);
  // calculate the mean variance for a bit counts array
  double variance() const;
};

// The arithmetic mean of the mean fingerprint bit variances for the
// fingerprints at each rgroup position.
double fingerprintVarianceScore(
    const std::vector<size_t> &bitCount,
    const std::vector<std::vector<RGroupMatch>> &matches,
    const std::set<int> &labels,
    std::map<int, std::shared_ptr<VarianceDataForLabel>> *labelsToVarianceData =
        nullptr);

// calculates fingerprint variance score from rgroup bit counts
double fingerprintVarianceGroupScore(
    const std::map<int, std::shared_ptr<VarianceDataForLabel>>
        &bitCountsByLabel);

// Adds a molecule match to the rgroup fingerprint bit counts
// vectors
void addVarianceData(
    int matchNumber, int permutationNumber,
    const std::vector<std::vector<RGroupMatch>> &matches,
    const std::set<int> &labels,
    std::map<int, std::shared_ptr<VarianceDataForLabel>> &labelsToVarianceData);

// Subtracts a molecule match from the rgroup fingerprint bit counts
// vectors
void removeVarianceData(
    int matchNumber, int permutationNumber,
    const std::vector<std::vector<RGroupMatch>> &matches,
    const std::set<int> &labels,
    std::map<int, std::shared_ptr<VarianceDataForLabel>> &labelsToVarianceData);

// Fingerprint score based on distance to fingerprint centroid for rgroups at
// each label
// Quite slow
double fingerprintDistanceScore(
    const std::vector<size_t> &bitCount,
    const std::vector<std::vector<RGroupMatch>> &matches,
    const std::set<int> &labels);

}  // namespace RDKit

#endif  // RDKIT_RGROUPFINGERPRINTSCORE_H
