//
//  Copyright (C) 2017 Novartis Institutes for BioMedical Research
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#ifndef RGROUP_DECOMP_DATA
#define RGROUP_DECOMP_DATA

#include "RGroupCore.h"
#include "RGroupDecomp.h"
#include "RGroupMatch.h"
#include "RGroupScore.h"
#include "RGroupFingerprintScore.h"
#include "RGroupGa.h"
#include <vector>
#include <map>

// #define VERBOSE 1

namespace RDKit {
struct RGroupDecompData {
  // matches[mol_idx] == vector of potential matches
  std::map<int, RCore> cores;
  std::map<std::string, int> newCores;  // new "cores" found along the way
  int newCoreLabel = EMPTY_CORE_LABEL;
  RGroupDecompositionParameters params;

  std::vector<std::vector<RGroupMatch>> matches;
  std::set<int> labels;
  std::vector<size_t> permutation;
  unsigned int pruneLength = 0U;
  FingerprintVarianceScoreData prunedFingerprintVarianceScoreData;
  std::map<int, std::vector<int>> userLabels;

  std::vector<int> processedRlabels;

  std::map<int, int> finalRlabelMapping;

  RGroupDecompData(const RWMol &inputCore,
                   RGroupDecompositionParameters inputParams)
      : params(std::move(inputParams)) {
    cores[0] = RCore(inputCore);
    prepareCores();
  }

  RGroupDecompData(const std::vector<ROMOL_SPTR> &inputCores,
                   RGroupDecompositionParameters inputParams)
      : params(std::move(inputParams)) {
    for (size_t i = 0; i < inputCores.size(); ++i) {
      cores[i] = RCore(*inputCores[i]);
    }

    prepareCores();
  }

  void prepareCores() {
    for (auto &core : cores) {
      RWMol *alignCore = core.first ? cores[0].core.get() : nullptr;
      BOOST_LOG(rdDebugLog) << "Preparing core " << core.first << std::endl;
      CHECK_INVARIANT(params.prepareCore(*core.second.core, alignCore),
                      "Could not prepare at least one core");
      if (params.onlyMatchAtRGroups) {
        core.second.findIndicesWithRLabel();
      }
      core.second.countUserRGroups();
      core.second.labelledCore.reset(new RWMol(*core.second.core));
    }
  }

  void setRlabel(Atom *atom, int rlabel) {
    PRECONDITION(rlabel != 0, "RLabels must be >0");
    if (params.rgroupLabelling & AtomMap) {
      atom->setAtomMapNum(rlabel);
    }

    if (params.rgroupLabelling & MDLRGroup) {
      std::string dLabel = "R" + std::to_string(rlabel);
      atom->setProp(common_properties::dummyLabel, dLabel);
      setAtomRLabel(atom, rlabel);
    }

    if (params.rgroupLabelling & Isotope) {
      atom->setIsotope(rlabel + 1);
    }
  }

  double scoreFromPrunedData(const std::vector<size_t> &permutation,
                             bool reset = true) {
    PRECONDITION(
        static_cast<RGroupScore>(params.scoreMethod) == FingerprintVariance,
        "Scoring method is not fingerprint variance!");

    PRECONDITION(permutation.size() >= pruneLength,
                 "Illegal permutation prune length");
    if (permutation.size() < pruneLength * 1.5) {
      for (unsigned int pos = pruneLength; pos < permutation.size(); pos++) {
        prunedFingerprintVarianceScoreData.addVarianceData(pos, permutation[pos], matches, labels);
      }
      double score =
          prunedFingerprintVarianceScoreData.fingerprintVarianceGroupScore();
      if (reset) {
        for (unsigned int pos = pruneLength; pos < permutation.size(); pos++) {
          prunedFingerprintVarianceScoreData.removeVarianceData(pos, permutation[pos], matches, labels);
        }
      } else {
        pruneLength = permutation.size();
      }
      return score;
    } else {
      if (reset) {
        return fingerprintVarianceScore(permutation, matches, labels);
      } else {
        prunedFingerprintVarianceScoreData.clear();
        pruneLength = permutation.size();
        return fingerprintVarianceScore(permutation, matches, labels,
                                        &prunedFingerprintVarianceScoreData);
      }
    }
  }

  void prune() {  // prune all but the current "best" permutation of matches
    for (size_t mol_idx = 0; mol_idx < permutation.size(); ++mol_idx) {
      std::vector<RGroupMatch> keepVector;
      keepVector.push_back(matches[mol_idx][permutation[mol_idx]]);
      matches[mol_idx] = keepVector;
    }

    permutation = std::vector<size_t>(matches.size(), 0);
    if (params.scoreMethod == FingerprintVariance &&
        params.matchingStrategy != GA) {
      scoreFromPrunedData(permutation, false);
    }
  }

  // Return the RGroups with the current "best" permutation
  //  of matches.
  std::vector<RGroupMatch> GetCurrentBestPermutation() const {
    const bool removeAllHydrogenRGroups = params.removeAllHydrogenRGroups;

    std::vector<RGroupMatch> results;  // std::map<int, RGroup> > result;
    for (size_t i = 0; i < permutation.size(); ++i) {
      CHECK_INVARIANT(i < matches.size(),
                      "Best Permutation mol idx out of range");
      CHECK_INVARIANT(permutation[i] < matches[i].size(),
                      "Selected match at permutation out of range");
      results.push_back(matches[i][permutation[i]]);
    }

    if (removeAllHydrogenRGroups) {
      // if a label is all hydrogens, remove it

      // This logic is a bit tricky, find all labels that have common cores
      //  and analyze those sets independently.
      //  i.e. if core 1 doesn't have R1 then don't analyze it in when looking
      //  at label 1
      std::map<int, std::set<int>> labelCores;  // map from label->cores
      std::set<int> coresVisited;
      for (auto &position : results) {
        int core_idx = position.core_idx;
        if (coresVisited.find(core_idx) == coresVisited.end()) {
          coresVisited.insert(core_idx);
          auto core = cores.find(core_idx);
          if (core != cores.end()) {
            for (auto rlabels : getRlabels(*core->second.core)) {
              int rlabel = rlabels.first;
              labelCores[rlabel].insert(core_idx);
            }
          }
        }
      }

      for (int label : labels) {
        bool allH = true;
        for (auto &position : results) {
          R_DECOMP::const_iterator rgroup = position.rgroups.find(label);
          bool labelHasCore = labelCores[label].find(position.core_idx) !=
                              labelCores[label].end();
          if (labelHasCore && rgroup != position.rgroups.end() &&
                               !rgroup->second->is_hydrogen) {
            allH = false;
            break;
          }
        }

        if (allH) {
          for (auto &position : results) {
            position.rgroups.erase(label);
          }
        }
      }
    }
    return results;
  }

  class UsedLabels {
   public:
    std::set<int> labels_used;
    bool add(int rlabel) {
      if (labels_used.find(rlabel) != labels_used.end()) {
        return false;
      }
      labels_used.insert(rlabel);
      return true;
    }

    int next() {
      int i = 1;
      while (labels_used.find(i) != labels_used.end()) {
        ++i;
      }
      labels_used.insert(i);
      return i;
    }
  };

  void addCoreUserLabels(const RWMol &core, std::set<int> &userLabels) {
    auto atoms = getRlabels(core);
    for (const auto &p : atoms) {
      if (p.first > 0) {
        userLabels.insert(p.first);
      }
    }
  }

  void relabelCore(RWMol &core, std::map<int, int> &mappings,
                   UsedLabels &used_labels, const std::set<int> &indexLabels,
                   std::map<int, std::vector<int>> extraAtomRLabels) {
    // Now remap to proper rlabel ids
    //  if labels are positive, they come from User labels
    //  if they are negative, they come from indices and should be
    //  numbered *after* the user labels.
    //
    //  Some indices are attached to multiple bonds,
    //   these rlabels should be incrementally added last
    std::map<int, Atom *> atoms = getRlabels(core);
    // a core only has one labelled index
    //  a secondary structure extraAtomRLabels contains the number
    //  of bonds between this atom and the side chain

    // a sidechain atom has a vector of the attachments back to the
    //  core that takes the place of numBondsToRlabel

    std::map<int, std::vector<int>> bondsToCore;
    std::vector<std::pair<Atom *, Atom *>> atomsToAdd;  // adds -R if necessary

    // Deal with user supplied labels
    for (auto rlabels : atoms) {
      int userLabel = rlabels.first;
      if (userLabel < 0) {
        continue;  // not a user specified label
      }
      Atom *atom = rlabels.second;
      mappings[userLabel] = userLabel;
      used_labels.add(userLabel);

      if (atom->getAtomicNum() == 0 &&
          atom->getDegree() == 1) {  // add to existing dummy/rlabel
        setRlabel(atom, userLabel);
      } else {  // adds new rlabel
        auto *newAt = new Atom(0);
        setRlabel(newAt, userLabel);
        atomsToAdd.emplace_back(atom, newAt);
      }
    }

    // Deal with non-user supplied labels
    for (auto newLabel : indexLabels) {
      auto atm = atoms.find(newLabel);
      if (atm == atoms.end()) {
        continue;
      }

      Atom *atom = atm->second;

      int rlabel;
      auto mapping = mappings.find(newLabel);
      if (mapping == mappings.end()) {
        rlabel = used_labels.next();
        mappings[newLabel] = rlabel;
      } else {
        rlabel = mapping->second;
      }

      if (atom->getAtomicNum() == 0 &&
          !isAnyAtomWithMultipleNeighborsOrNotUserRLabel(
              *atom)) {  // add to dummy
        setRlabel(atom, rlabel);
      } else {
        auto *newAt = new Atom(0);
        setRlabel(newAt, rlabel);
        atomsToAdd.emplace_back(atom, newAt);
      }
    }

    // Deal with multiple bonds to the same label
    for (auto &extraAtomRLabel : extraAtomRLabels) {
      auto atm = atoms.find(extraAtomRLabel.first);
      if (atm == atoms.end()) {
        continue;  // label not used in the rgroup
      }
      Atom *atom = atm->second;

      for (int &i : extraAtomRLabel.second) {
        int rlabel = used_labels.next();
        i = rlabel;
        // Is this necessary?
        CHECK_INVARIANT(
            atom->getAtomicNum() > 1,
            "Multiple attachments to a dummy (or hydrogen) is weird.");
        auto *newAt = new Atom(0);
        setRlabel(newAt, rlabel);
        atomsToAdd.emplace_back(atom, newAt);
      }
    }

    for (auto &i : atomsToAdd) {
      core.addAtom(i.second, false, true);
      core.addBond(i.first, i.second, Bond::SINGLE);
      MolOps::setHydrogenCoords(&core, i.second->getIdx(), i.first->getIdx());
    }
    core.updatePropertyCache(false);  // this was github #1550
  }

  void relabelRGroup(RGroupData &rgroup, const std::map<int, int> &mappings) {
    PRECONDITION(rgroup.combinedMol.get(), "Unprocessed rgroup");

    RWMol &mol = *rgroup.combinedMol.get();

    if (rgroup.combinedMol->hasProp(done)) {
      rgroup.labelled = true;
      return;
    }

    mol.setProp(done, true);
    std::vector<std::pair<Atom *, Atom *>> atomsToAdd;  // adds -R if necessary
    std::map<int, int> rLabelCoreIndexToAtomicWt;

    for (RWMol::AtomIterator atIt = mol.beginAtoms(); atIt != mol.endAtoms();
         ++atIt) {
      Atom *atom = *atIt;
      if (atom->hasProp(SIDECHAIN_RLABELS)) {
        atom->setIsotope(0);
        const std::vector<int> &rlabels =
            atom->getProp<std::vector<int>>(SIDECHAIN_RLABELS);
        // switch on atom mappings or rlabels....

        for (int rlabel : rlabels) {
          auto label = mappings.find(rlabel);
          CHECK_INVARIANT(label != mappings.end(), "Unprocessed mapping");

          if (atom->getAtomicNum() == 0) {
            setRlabel(atom, label->second);
          } else if (atom->hasProp(RLABEL_CORE_INDEX)) {
            atom->setAtomicNum(0);
            setRlabel(atom, label->second);
          } else {
            auto *newAt = new Atom(0);
            setRlabel(newAt, label->second);
            atomsToAdd.emplace_back(atom, newAt);
          }
        }
      }
      if (atom->hasProp(RLABEL_CORE_INDEX)) {
        // convert to dummy as we don't want to collapse hydrogens onto the core
        // match
        auto rLabelCoreIndex = atom->getProp<int>(RLABEL_CORE_INDEX);
        rLabelCoreIndexToAtomicWt[rLabelCoreIndex] = atom->getAtomicNum();
        atom->setAtomicNum(0);
      }
    }

    for (auto &i : atomsToAdd) {
      mol.addAtom(i.second, false, true);
      mol.addBond(i.first, i.second, Bond::SINGLE);
      MolOps::setHydrogenCoords(&mol, i.second->getIdx(), i.first->getIdx());
    }

    if (params.removeHydrogensPostMatch) {
      RDLog::BlockLogs blocker;
      bool implicitOnly = false;
      bool updateExplicitCount = false;
      bool sanitize = false;
      MolOps::removeHs(mol, implicitOnly, updateExplicitCount, sanitize);
    }

    mol.updatePropertyCache(false);  // this was github #1550
    rgroup.labelled = true;

    // Restore any core matches that we have set to dummy
    for (RWMol::AtomIterator atIt = mol.beginAtoms(); atIt != mol.endAtoms();
         ++atIt) {
      Atom *atom = *atIt;
      if (atom->hasProp(RLABEL_CORE_INDEX)) {
        // don't need to set IsAromatic on atom - that seems to have been saved
        atom->setAtomicNum(
            rLabelCoreIndexToAtomicWt[atom->getProp<int>(RLABEL_CORE_INDEX)]);
        atom->setNoImplicit(true);
      }
    }

#ifdef VERBOSE
    std::cerr << "Relabel Rgroup smiles " << MolToSmiles(mol) << std::endl;
#endif
  }

  // relabel the core and sidechains using the specified user labels
  //  if matches exist for non labelled atoms, these are added as well
  void relabel() {
    std::vector<RGroupMatch> best = GetCurrentBestPermutation();

    // get the labels used
    std::set<int> userLabels;
    std::set<int> indexLabels;

    // Go through all the RGroups and find out which labels were
    //  actually used.

    // some atoms will have multiple attachment points, i.e. cycles
    //  split these up into new rlabels if necessary
    //  These are detected at match time
    //  This vector will hold the extra (new) labels required
    std::map<int, std::vector<int>> extraAtomRLabels;

    for (auto &it : best) {
      for (auto &rgroup : it.rgroups) {
        if (rgroup.first >= 0) {
          userLabels.insert(rgroup.first);
        }
        if (rgroup.first < 0 && !params.onlyMatchAtRGroups) {
          indexLabels.insert(rgroup.first);
        }

        std::map<int, int> rlabelsUsedInRGroup =
            rgroup.second->getNumBondsToRlabels();
        for (auto &numBondsUsed : rlabelsUsedInRGroup) {
          // Make space for the extra labels
          if (numBondsUsed.second > 1) {  // multiple rgroup bonds to same atom
            extraAtomRLabels[numBondsUsed.first].resize(numBondsUsed.second -
                                                        1);
          }
        }
      }
    }

    // find user labels that are not present in the decomposition
    for (auto &core : cores) {
      core.second.labelledCore.reset(new RWMol(*core.second.core));
      addCoreUserLabels(*core.second.labelledCore, userLabels);
    }

    // Assign final RGroup labels to the cores and propagate these to
    //  the scaffold
    finalRlabelMapping.clear();

    UsedLabels used_labels;
    // Add all the user labels now to prevent an index label being assigned to a
    // user label when multiple cores are present (e.g. the user label is
    // present in the second core, but not the first).
    for (auto userLabel : userLabels) {
      used_labels.add(userLabel);
    }
    for (auto &core : cores) {
      relabelCore(*core.second.labelledCore, finalRlabelMapping, used_labels,
                  indexLabels, extraAtomRLabels);
    }

    for (auto &it : best) {
      for (auto &rgroup : it.rgroups) {
        relabelRGroup(*rgroup.second, finalRlabelMapping);
      }
    }

    std::set<int> uniqueMappedValues;
    std::transform(finalRlabelMapping.cbegin(), finalRlabelMapping.cend(),
                   std::inserter(uniqueMappedValues, uniqueMappedValues.end()),
                   [](const std::pair<int, int> &p) { return p.second; });
    CHECK_INVARIANT(finalRlabelMapping.size() == uniqueMappedValues.size(),
                    "Error in uniqueness of final RLabel mapping");
    CHECK_INVARIANT(
        uniqueMappedValues.size() == userLabels.size() + indexLabels.size(),
        "Error in final RMapping size");
  }

  // compute the number of rgroups that would be added if we
  //  accepted this permutation
  size_t compute_num_added_rgroups(const std::vector<size_t> &tied_permutation,
                                   const std::vector<int> &ordered_labels,
                                   std::vector<int> &heavy_counts,
                                   size_t &number_user_rgroups_matched) {
    // heavy_counts is a vector which has the same size of labels
    // for each label we add an increment if a molecule
    // bears an R-group at that label. The increment has opposite
    // sign to the label
    size_t i = 0;
    size_t num_added_rgroups = 0;
    heavy_counts.resize(labels.size(), 0);
    // number_user_rgroups_matched counts the total number of user labelled r
    // groups filled in this permutation.  We want to maximize this number
    number_user_rgroups_matched = 0;

    for (int label : ordered_labels) {
      bool incremented = false;
      for (size_t m = 0; m < tied_permutation.size();
           ++m) {  // for each molecule
        // for each molecule, check if we add an R-group at this negative label
        // if we do, count it once. So we know how many different negative
        // labels we have filled: we prefer permutations which fill less, as it
        // means we have added less groups on different positions
        auto rg = matches[m][tied_permutation[m]].rgroups.find(label);
        if (rg != matches[m][tied_permutation[m]].rgroups.end() &&
            !rg->second->is_hydrogen) {
          if (label < 0 && !incremented) {
            incremented = true;
            ++num_added_rgroups;
          } else if (label > 0) {
            ++number_user_rgroups_matched;
          }
          ++heavy_counts[i];
        }
      }
      ++i;
    }
    return num_added_rgroups;
  }

  double score(const std::vector<size_t> &permutation,
               FingerprintVarianceScoreData *fingerprintVarianceScoreData =
                   nullptr) const {
    RGroupScore scoreMethod = static_cast<RGroupScore>(params.scoreMethod);
    switch (scoreMethod) {
      case Match:
        return matchScore(permutation, matches, labels);
        break;
      case FingerprintVariance:
        return fingerprintVarianceScore(permutation, matches, labels,
                                        fingerprintVarianceScoreData);
        break;
      default:;
    }
    return NAN;
  }

  RGroupDecompositionProcessResult process(bool pruneMatches,
                                           bool finalize = false) {
    if (matches.empty()) {
      return RGroupDecompositionProcessResult(false, -1);
    }
    auto t0 = std::chrono::steady_clock::now();
    std::vector<size_t> best_permutation;
    std::vector<std::vector<size_t>> ties;
    double best_score = -std::numeric_limits<double>::max();
    std::unique_ptr<CartesianProduct> iterator;

    if (params.matchingStrategy == GA) {
      RGroupGa ga(*this, params.timeout >= 0 ? &t0 : nullptr);
      if (ga.numberPermutations() < 10000) {
        params.matchingStrategy = Exhaustive;
      } else {
        if (params.gaNumberRuns > 1) {
          auto results = ga.runBatch();
          auto best = max_element(results.begin(), results.end(),
                                  [](const GaResult &a, const GaResult &b) {
                                    return a.score < b.score;
                                  });
          ties = best->permutations;
          best_score = best->score;
        } else {
          auto result = ga.run();
          ties = result.permutations;
          best_score = result.score;
        }
        best_permutation = ties[0];
      }
    }
    if (params.matchingStrategy != GA) {
      // Exhaustive search, get the MxN matrix
      // (M = matches.size(): number of molecules
      //  N = iterator.maxPermutations)
      std::vector<size_t> permutations;

      std::transform(
          matches.begin(), matches.end(), std::back_inserter(permutations),
          [](const std::vector<RGroupMatch> &m) { return m.size(); });
      permutation = std::vector<size_t>(permutations.size(), 0);

      // run through all possible matches and score each
      //  set
      best_permutation = permutation;

      size_t count = 0;
#ifdef DEBUG
      std::cerr << "Processing" << std::endl;
#endif
      std::unique_ptr<CartesianProduct> it(new CartesianProduct(permutations));
      iterator = std::move(it);
      // Iterates through the permutation idx, i.e.
      //  [m1_permutation_idx,  m2_permutation_idx, m3_permutation_idx]

      while (iterator->next()) {
        if (count > iterator->maxPermutations) {
          throw ValueErrorException("next() did not finish");
        }
#ifdef DEBUG
        std::cerr << "**************************************************"
                  << std::endl;
#endif
        double newscore = params.scoreMethod == FingerprintVariance
                              ? scoreFromPrunedData(iterator->permutation)
                              : score(iterator->permutation);

        if (fabs(newscore - best_score) <
            1e-6) {  // heuristic to overcome floating point comparison issues
          ties.push_back(iterator->permutation);
        } else if (newscore > best_score) {
#ifdef DEBUG
          std::cerr << " ===> current best:" << newscore << ">" << best_score
                    << std::endl;
#endif
          ties.clear();
          ties.push_back(iterator->permutation);
          best_score = newscore;
          best_permutation = iterator->permutation;
          iterator->value(best_permutation);
        }
        ++count;
      }

      BOOST_LOG(rdDebugLog)
          << "Exhaustive or GreedyChunks process, best score " << best_score
          << " permutation size " << best_permutation.size() << std::endl;
    }

    if (ties.size() > 1) {
      size_t max_perm_value = 0;
      size_t smallest_added_rgroups = labels.size();
      size_t largest_sum_user_rgroups = -1;
      std::vector<int> largest_heavy_counts(labels.size(), 0);
      std::vector<int> ordered_labels;
      std::copy_if(labels.begin(), labels.end(),
                   std::back_inserter(ordered_labels),
                   [](const int &i) { return !(i < 0); });
      std::copy_if(labels.begin(), labels.end(),
                   std::back_inserter(ordered_labels),
                   [](const int &i) { return (i < 0); });
      for (const auto &tied_permutation : ties) {
        std::vector<int> heavy_counts;
        size_t number_user_rgroups_matched = 0;
        size_t num_added_rgroups = compute_num_added_rgroups(
            tied_permutation, ordered_labels, heavy_counts,
            number_user_rgroups_matched);
        size_t perm_value =
            iterator ? iterator->value(tied_permutation) : max_perm_value;
        if (number_user_rgroups_matched > largest_sum_user_rgroups) {
          largest_sum_user_rgroups = number_user_rgroups_matched;
          smallest_added_rgroups = num_added_rgroups;
          largest_heavy_counts = heavy_counts;
          max_perm_value = perm_value;
          best_permutation = tied_permutation;
        } else if (num_added_rgroups < smallest_added_rgroups) {
          smallest_added_rgroups = num_added_rgroups;
          largest_heavy_counts = heavy_counts;
          max_perm_value = perm_value;
          best_permutation = tied_permutation;
        } else if (num_added_rgroups == smallest_added_rgroups) {
          if (heavy_counts > largest_heavy_counts) {
            largest_heavy_counts = heavy_counts;
            max_perm_value = perm_value;
            best_permutation = tied_permutation;
          } else if (heavy_counts == largest_heavy_counts) {
            if (perm_value > max_perm_value) {
              max_perm_value = perm_value;
              best_permutation = tied_permutation;
            }
          }
        }
        checkForTimeout(t0, params.timeout);
      }
    }
    permutation = best_permutation;
    if (pruneMatches || finalize) {
      prune();
    }

    if (finalize) {
      relabel();
    }

    return RGroupDecompositionProcessResult(true, best_score);
  }
};
}  // namespace RDKit

#endif
