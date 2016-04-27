//
// Copyright (c) 2016 Greg Landrum
//
//  @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#ifndef RD_MULTIFPBREADER_H_APR2016
#define RD_MULTIFPBREADER_H_APR2016
/*! \file MultiFPBReader.h

  \brief contains a class for reading and searching collections of FPB files

  \b Note that this functionality is experimental and the API may change
     in future releases.
*/

#include <RDGeneral/Exceptions.h>
#include <DataStructs/ExplicitBitVect.h>
#include <DataStructs/FPBReader.h>
#include <boost/tuple/tuple.hpp>

namespace RDKit {

//! class for reading and searching multiple FPB files
/*!
  basic usage:
  \code
  FPBReader r1("foo1.fpb"),r2("foo2.fpb");
  std::vector<FPBReader *> readers;
  readers.append(&r1);
  readers.append(&r2);
  MultiFPBReader fpbs(readers);
  fpbs.init();
  boost::shared_ptr<ExplicitBitVect> ebv = fpbs.getReader(0)->getFP(95);
  std::vector<boost::tuple<double,unsigned int, unsigned int> > nbrs =
      fpbs.getTanimotoNeighbors(*ebv.get(), 0.70);
  \endcode

  \b Note: this functionality is experimental and the API may change
     in future releases.

  <b>Note on thread safety</b>
  Operations that involve reading from FPB files are not thread safe.
  This means that the \c init() method is not thread safe and none of the
  search operations are thread safe when an \c FPBReader is initialized in
  \c lazyRead mode.

*/
class MultiFPBReader {
 public:
  typedef boost::tuple<double, unsigned int, unsigned int> ResultTuple;
  MultiFPBReader() : df_init(false){};
  MultiFPBReader(std::vector<FPBReader *> &readers);

  ~MultiFPBReader() { df_init = false; };

  //! Read the data from the file and initialize internal data structures
  /*!
  This must be called before most of the other methods of this clases.
  It calls the \c init() method on each of the child FPBReaders

  */
  void init();

  //! returns the number of readers
  unsigned int length() const { return d_readers.size(); };
  //! returns the number of bits in our fingerprints (all readers are expected
  //! to have the same length)
  unsigned int nBits() const;

  //! returns a particular reader
  /*!

    \param which: the reader to return

  */
  FPBReader *getReader(unsigned int which);

  //! returns tanimoto neighbors that are within a similarity threshold
  /*!
  The result vector of (similarity,index,reader) tuples is sorted in order
  of decreasing similarity

    \param bv the query fingerprint
    \param threshold the minimum similarity to return

  */
  std::vector<ResultTuple> getTanimotoNeighbors(const boost::uint8_t *bv,
                                                double threshold = 0.7) const;
  //! \overload
  std::vector<ResultTuple> getTanimotoNeighbors(
      boost::shared_array<boost::uint8_t> bv, double threshold = 0.7) const {
    return getTanimotoNeighbors(bv.get(), threshold);
  };
  //! \overload
  std::vector<ResultTuple> getTanimotoNeighbors(const ExplicitBitVect &ebv,
                                                double threshold = 0.7) const;

  //! returns Tversky neighbors that are within a similarity threshold
  /*!
  The result vector of (similarity,index) pairs is sorted in order
  of decreasing similarity

    \param bv the query fingerprint
    \param ca the Tversky a coefficient
    \param cb the Tversky a coefficient
    \param threshold the minimum similarity to return

  */
  std::vector<ResultTuple> getTverskyNeighbors(const boost::uint8_t *bv,
                                               double ca, double cb,
                                               double threshold = 0.7) const;
  //! \overload
  std::vector<ResultTuple> getTverskyNeighbors(
      boost::shared_array<boost::uint8_t> bv, double ca, double cb,
      double threshold = 0.7) const {
    return getTverskyNeighbors(bv.get(), ca, cb, threshold);
  };
  //! \overload
  std::vector<ResultTuple> getTverskyNeighbors(const ExplicitBitVect &ebv,
                                               double ca, double cb,
                                               double threshold = 0.7) const;

  //! returns indices of all fingerprints that completely contain this one
  /*! (i.e. where all the bits set in the query are also set in the db
   molecule)
   */
  std::vector<std::pair<unsigned int, unsigned int> > getContainingNeighbors(
      const boost::uint8_t *bv) const;
  //! \overload
  std::vector<std::pair<unsigned int, unsigned int> > getContainingNeighbors(
      boost::shared_array<boost::uint8_t> bv) const {
    return getContainingNeighbors(bv.get());
  };
  //! \overload
  std::vector<std::pair<unsigned int, unsigned int> > getContainingNeighbors(
      const ExplicitBitVect &ebv) const;

 private:
  std::vector<FPBReader *> d_readers;
  bool df_init;

  // disable automatic copy constructors and assignment operators
  // for this class and its subclasses.  They will likely be
  // carrying around stream pointers and copying those is a recipe
  // for disaster.
  MultiFPBReader(const MultiFPBReader &);
  MultiFPBReader &operator=(const MultiFPBReader &);
};
}
#endif
