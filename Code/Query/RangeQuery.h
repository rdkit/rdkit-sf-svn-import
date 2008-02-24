//
// Copyright (c) 2003-2008 Greg Landrum and Rational Discovery LLC
//
//   @@ All Rights Reserved  @@
//
#ifndef __RD_RANGEQUERY_H__
#define __RD_RANGEQUERY_H__
#include "Query.h"
#include <utility>

namespace Queries {

  //! \brief a Query implementing a range: arguments must 
  //!  fall in a particular range of values.
  //!
  //!  The ends of the range default to be open, but they can
  //!  individually set to be closed.  
  //!
  //!  There is also an optional tolerance to be used in comparisons
  template <class MatchFuncArgType, class DataFuncArgType=MatchFuncArgType,
    bool needsConversion=false>
  class RangeQuery :
    public Query<MatchFuncArgType, DataFuncArgType,needsConversion> {

  public:
    RangeQuery() : d_upper(0), d_lower(0), d_tol(0), df_upperOpen(true), df_lowerOpen(true){
      this->df_negate = false;
    };
    //! construct and set the lower and upper bounds
    RangeQuery(DataFuncArgType lower,DataFuncArgType upper) : d_upper(upper), d_lower(lower), d_tol(0), df_upperOpen(true), df_lowerOpen(true){
      this->df_negate = false;
    };

    //! sets our upper bound
    void setUpper (DataFuncArgType what) { this->d_upper = what; };
    //! returns our upper bound
    const DataFuncArgType getUpper() const { return this->d_upper; };
    //! sets our lower bound
    void setLower (DataFuncArgType what) { this->d_lower = what; };
    //! returns our lower bound
    const DataFuncArgType getLower() const { return this->d_lower; };

    //! sets whether or not the ends of the range are open
    void setEndsOpen(bool lower, bool upper) {
      this->df_lowerOpen = lower;
      this->df_upperOpen = upper;
    };
    //! returns the state of our ends (open or not)
    std::pair<bool,bool> getEndsOpen() {
      return std::make_pair(this->df_lowerOpen,this->df_upperOpen);
    };
  
    //! sets our tolerance
    void setTol(DataFuncArgType what) { this->d_tol = what; };
    //! returns our tolerance
    const DataFuncArgType getTol() const { return this->d_tol; };
  
    bool Match(const DataFuncArgType what) const {
      MatchFuncArgType mfArg = TypeConvert(what,Int2Type<needsConversion>());
      int lCmp = queryCmp(this->d_lower,mfArg,this->d_tol);
      int uCmp = queryCmp(this->d_upper,mfArg,this->d_tol);
      bool lowerRes,upperRes;
      if( this->df_lowerOpen ) lowerRes = lCmp < 0;
      else lowerRes = lCmp <= 0;
      if( this->df_upperOpen ) upperRes = uCmp > 0;
      else upperRes = uCmp >= 0;

      bool tempR = !(lowerRes && upperRes);
      if( this->getNegation() ) return tempR;
      else return !tempR;
    };

    Query<MatchFuncArgType,DataFuncArgType,needsConversion> *
    copy( ) const {
      RangeQuery<MatchFuncArgType,DataFuncArgType,needsConversion> *res =
	new RangeQuery<MatchFuncArgType,DataFuncArgType,needsConversion>();
      res->setUpper(this->d_upper);
      res->setLower(this->d_lower);
      res->setTol(this->d_tol);
      res->setNegation(this->getNegation());
      res->setEndsOpen(this->df_lowerOpen,this->df_upperOpen);
      res->setDataFunc(this->d_dataFunc);
      res->d_description = this->d_description;
      return res;
    };

  protected:
    DataFuncArgType d_upper,d_lower;
    DataFuncArgType d_tol;
    bool df_upperOpen,df_lowerOpen;
  private:    
    friend class boost::serialization::access;
    //! required by boost::serialization, handles both serialization and
    //! deserialization
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object<Query<MatchFuncArgType,DataFuncArgType,needsConversion> >(*this);
      ar & d_upper;
      ar & d_lower;
      ar & d_tol;
      ar & df_upperOpen;
      ar & df_lowerOpen;
    }

  };

}
#endif
