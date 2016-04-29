//
// Copyright (c) 2016 Greg Landrum
//
//  @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#include <boost/tuple/tuple_comparison.hpp>

#include <RDGeneral/Invariant.h>
#include <RDGeneral/Ranking.h>
#include <RDGeneral/RDThreads.h>

#include <boost/foreach.hpp>
#include "MultiFPBReader.h"
#include <algorithm>

namespace RDKit {

namespace detail {
boost::uint8_t *bitsetToBytes(const boost::dynamic_bitset<> &bitset);
}

namespace {
struct tplSorter
    : public std::binary_function<MultiFPBReader::ResultTuple,
                                  MultiFPBReader::ResultTuple, bool> {
  bool operator()(const MultiFPBReader::ResultTuple &v1,
                  const MultiFPBReader::ResultTuple &v2) const {
    if (v1.get<0>() == v2.get<0>()) {
      if (v1.get<2>() == v2.get<2>()) {
        return v1.get<1>() < v2.get<1>();
      } else {
        return v1.get<2>() < v2.get<2>();
      }
    } else {
      return v1.get<0>() > v2.get<0>();
    }
  }
};
struct pairSorter
    : public std::binary_function<std::pair<unsigned int, unsigned int>,
                                  std::pair<unsigned int, unsigned int>, bool> {
  bool operator()(const std::pair<unsigned int, unsigned int> &v1,
                  const std::pair<unsigned int, unsigned int> &v2) const {
    if (v1.first == v2.first) {
      return v1.second < v2.second;
    } else {
      return v1.first < v2.first;
    }
  }
};

struct sim_args {
  const boost::uint8_t *bv;
  double ca, cb;
  double threshold;
  const std::vector<FPBReader *> &readers;
  std::vector<std::vector<MultiFPBReader::ResultTuple> > *res;
};

void tani_helper(unsigned int threadId, unsigned int numThreads,
                 sim_args *args) {
  for (unsigned int i = threadId; i < args->readers.size(); i += numThreads) {
    std::vector<std::pair<double, unsigned int> > r_res =
        args->readers[i]->getTanimotoNeighbors(args->bv, args->threshold);
    (*args->res)[i].clear();
    (*args->res)[i].reserve(r_res.size());
    for (std::vector<std::pair<double, unsigned int> >::const_iterator rit =
             r_res.begin();
         rit != r_res.end(); ++rit) {
      (*args->res)[i].push_back(
          MultiFPBReader::ResultTuple(rit->first, rit->second, i));
    }
  }
}

void get_tani_nbrs(const std::vector<FPBReader *> &d_readers,
                   const boost::uint8_t *bv, double threshold,
                   std::vector<MultiFPBReader::ResultTuple> &res,
                   int numThreads) {
  res.clear();
  res.resize(0);
  numThreads = getNumThreadsToUse(numThreads);
#ifdef RDK_THREADSAFE_SSS
  boost::thread_group tg;
#endif
  std::vector<std::vector<MultiFPBReader::ResultTuple> > accum(
      d_readers.size());
  sim_args args = {bv, 0., 0., threshold, d_readers, &accum};
  if (numThreads == 1) {
    tani_helper(0, 1, &args);
  }
#ifdef RDK_THREADSAFE_SSS
  else {
    for (unsigned int tid = 0; tid < numThreads && tid < d_readers.size();
         ++tid) {
      tg.add_thread(new boost::thread(tani_helper, tid, numThreads, &args));
    }
    tg.join_all();
  }
#endif

  for (unsigned int i = 0; i < d_readers.size(); ++i) {
    res.reserve(res.size() + accum[i].size());
    res.insert(res.end(), accum[i].begin(), accum[i].end());
  }
  std::sort(res.begin(), res.end(), tplSorter());
}

void get_tversky_nbrs(const std::vector<FPBReader *> &d_readers,
                      const boost::uint8_t *bv, double a, double b,
                      double threshold,
                      std::vector<MultiFPBReader::ResultTuple> &res,
                      int numThreads) {
  numThreads = getNumThreadsToUse(numThreads);
  res.clear();
  for (unsigned int i = 0; i < d_readers.size(); ++i) {
    std::vector<std::pair<double, unsigned int> > r_res =
        d_readers[i]->getTverskyNeighbors(bv, a, b, threshold);
    for (std::vector<std::pair<double, unsigned int> >::const_iterator rit =
             r_res.begin();
         rit != r_res.end(); ++rit) {
      res.push_back(MultiFPBReader::ResultTuple(rit->first, rit->second, i));
    }
  }
  std::sort(res.begin(), res.end(), tplSorter());
}

void get_containing_nbrs(
    const std::vector<FPBReader *> &d_readers, const boost::uint8_t *bv,
    std::vector<std::pair<unsigned int, unsigned int> > &res, int numThreads) {
  numThreads = getNumThreadsToUse(numThreads);
  res.clear();
  for (unsigned int i = 0; i < d_readers.size(); ++i) {
    std::vector<unsigned int> r_res = d_readers[i]->getContainingNeighbors(bv);
    for (std::vector<unsigned int>::const_iterator rit = r_res.begin();
         rit != r_res.end(); ++rit) {
      res.push_back(std::make_pair(*rit, i));
    }
  }
  std::sort(res.begin(), res.end(), pairSorter());
}

}  // end of anonymous namespace

void MultiFPBReader::init() {
  unsigned int nBits = 0;
  BOOST_FOREACH (FPBReader *rdr, d_readers) {
    rdr->init();
    if (!nBits) {
      nBits = rdr->nBits();
    } else {
      if (rdr->nBits() != nBits)
        throw ValueErrorException("bit lengths of child readers don't match");
    }
    df_init = true;
  }
};

MultiFPBReader::MultiFPBReader(std::vector<FPBReader *> &readers) {
  df_init = false;
  BOOST_FOREACH (FPBReader *rdr, readers) {
    PRECONDITION(rdr != NULL, "bad reader");
  }
  d_readers = readers;
}

FPBReader *MultiFPBReader::getReader(unsigned int which) {
  URANGE_CHECK(which, d_readers.size());
  return d_readers[which];
}
unsigned int MultiFPBReader::nBits() const {
  PRECONDITION(d_readers.size(), "no readers");
  PRECONDITION(df_init, "not initialized");
  return d_readers[0]->nBits();
}

std::vector<MultiFPBReader::ResultTuple> MultiFPBReader::getTanimotoNeighbors(
    const boost::uint8_t *bv, double threshold, int numThreads) const {
  PRECONDITION(df_init, "not initialized");

  std::vector<MultiFPBReader::ResultTuple> res;
  get_tani_nbrs(d_readers, bv, threshold, res, numThreads);
  return res;
}

std::vector<MultiFPBReader::ResultTuple> MultiFPBReader::getTanimotoNeighbors(
    const ExplicitBitVect &ebv, double threshold, int numThreads) const {
  PRECONDITION(df_init, "not initialized");
  std::vector<MultiFPBReader::ResultTuple> res;
  boost::uint8_t *bv = detail::bitsetToBytes(*(ebv.dp_bits));
  get_tani_nbrs(d_readers, bv, threshold, res, numThreads);
  delete[] bv;
  return res;
}

std::vector<MultiFPBReader::ResultTuple> MultiFPBReader::getTverskyNeighbors(
    const boost::uint8_t *bv, double ca, double cb, double threshold,
    int numThreads) const {
  PRECONDITION(df_init, "not initialized");
  std::vector<MultiFPBReader::ResultTuple> res;
  get_tversky_nbrs(d_readers, bv, ca, cb, threshold, res, numThreads);
  return res;
}

std::vector<MultiFPBReader::ResultTuple> MultiFPBReader::getTverskyNeighbors(
    const ExplicitBitVect &ebv, double ca, double cb, double threshold,
    int numThreads) const {
  PRECONDITION(df_init, "not initialized");
  std::vector<MultiFPBReader::ResultTuple> res;
  boost::uint8_t *bv = detail::bitsetToBytes(*(ebv.dp_bits));
  get_tversky_nbrs(d_readers, bv, ca, cb, threshold, res, numThreads);
  delete[] bv;
  return res;
}

std::vector<std::pair<unsigned int, unsigned int> >
MultiFPBReader::getContainingNeighbors(const boost::uint8_t *bv,
                                       int numThreads) const {
  PRECONDITION(df_init, "not initialized");
  std::vector<std::pair<unsigned int, unsigned int> > res;
  get_containing_nbrs(d_readers, bv, res, numThreads);
  return res;
}

std::vector<std::pair<unsigned int, unsigned int> >
MultiFPBReader::getContainingNeighbors(const ExplicitBitVect &ebv,
                                       int numThreads) const {
  PRECONDITION(df_init, "not initialized");
  std::vector<std::pair<unsigned int, unsigned int> > res;
  boost::uint8_t *bv = detail::bitsetToBytes(*(ebv.dp_bits));
  get_containing_nbrs(d_readers, bv, res, numThreads);
  delete[] bv;
  return res;
}

}  // end of RDKit namespace
