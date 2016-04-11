/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file Values.h
 * @author Richard Roberts
 *
 * @brief A non-templated config holding any types of Manifold-group elements
 *
 *  Detailed story:
 *  A values structure is a map from keys to values. It is used to specify the value of a bunch
 *  of variables in a factor graph. A Values is a values structure which can hold variables that
 *  are elements on manifolds, not just vectors. It then, as a whole, implements a aggregate type
 *  which is also a manifold element, and hence supports operations dim, retract, and localCoordinates.
 */

#pragma once

#include <utility>

#include <boost/foreach.hpp>

#include <gtsam/base/DerivedValue.h>
#include <gtsam/nonlinear/Values.h> // Only so Eclipse finds class definition

namespace gtsam {


  /* ************************************************************************* */
  template<class ValueType>
  struct _ValuesKeyValuePair {
    const Key key; ///< The key
    ValueType& value;  ///< The value

    _ValuesKeyValuePair(Key _key, ValueType& _value) : key(_key), value(_value) {}
  };

  /* ************************************************************************* */
  template<class ValueType>
  struct _ValuesConstKeyValuePair {
    const Key key; ///< The key
    const ValueType& value;  ///< The value

    _ValuesConstKeyValuePair(Key _key, const ValueType& _value) :
        key(_key), value(_value) {
    }
    _ValuesConstKeyValuePair(const _ValuesKeyValuePair<ValueType>& rhs) :
        key(rhs.key), value(rhs.value) {
    }
  };

  /* ************************************************************************* */

  // Cast helpers for making _Values[Const]KeyValuePair's from Values::[Const]KeyValuePair
  // need to use a struct here for later partial specialization
  template<class ValueType, class CastedKeyValuePairType, class KeyValuePairType>
  struct ValuesCastHelper {
  static CastedKeyValuePairType cast(KeyValuePairType key_value) {
    // Static cast because we already checked the type during filtering
    return CastedKeyValuePairType(key_value.key,
        const_cast<GenericValue<ValueType>&>(static_cast<const GenericValue<
            ValueType>&>(key_value.value)).value());
  }
  };
  // partial specialized version for ValueType == Value
  template<class CastedKeyValuePairType, class KeyValuePairType>
  struct ValuesCastHelper<Value, CastedKeyValuePairType, KeyValuePairType> {
    static CastedKeyValuePairType cast(KeyValuePairType key_value) {
      // Static cast because we already checked the type during filtering
      // in this case the casted and keyvalue pair are essentially the same type
      // (key, Value&) so perhaps this could be done with just a cast of the key_value?
      return CastedKeyValuePairType(key_value.key, key_value.value);
    }
  };
  // partial specialized version for ValueType == Value
  template<class CastedKeyValuePairType, class KeyValuePairType>
  struct ValuesCastHelper<const Value, CastedKeyValuePairType, KeyValuePairType> {
    static CastedKeyValuePairType cast(KeyValuePairType key_value) {
      // Static cast because we already checked the type during filtering
      // in this case the casted and keyvalue pair are essentially the same type
      // (key, Value&) so perhaps this could be done with just a cast of the key_value?
      return CastedKeyValuePairType(key_value.key, key_value.value);
    }
  };

  /* ************************************************************************* */
  template<class ValueType>
  class Values::Filtered {
  public:
    /** A key-value pair, with the value a specific derived Value type. */
    typedef _ValuesKeyValuePair<ValueType> KeyValuePair;
    typedef _ValuesConstKeyValuePair<ValueType> ConstKeyValuePair;
    typedef KeyValuePair value_type;

    typedef
      boost::transform_iterator<
      KeyValuePair(*)(Values::KeyValuePair),
      boost::filter_iterator<
      boost::function<bool(const Values::ConstKeyValuePair&)>,
      Values::iterator> >
      iterator;

    typedef iterator const_iterator;

    typedef
      boost::transform_iterator<
      ConstKeyValuePair(*)(Values::ConstKeyValuePair),
      boost::filter_iterator<
      boost::function<bool(const Values::ConstKeyValuePair&)>,
      Values::const_iterator> >
      const_const_iterator;

    iterator begin() { return begin_; }
    iterator end() { return end_; }
    const_iterator begin() const { return begin_; }
    const_iterator end() const { return end_; }
    const_const_iterator beginConst() const { return constBegin_; }
    const_const_iterator endConst() const { return constEnd_; }

    /** Returns the number of values in this view */
    size_t size() const {
      size_t i = 0;
      for (const_const_iterator it = beginConst(); it != endConst(); ++it)
        ++i;
      return i;
    }

  private:
    Filtered(
        const boost::function<bool(const Values::ConstKeyValuePair&)>& filter,
        Values& values) :
        begin_(
            boost::make_transform_iterator(
                boost::make_filter_iterator(filter, values.begin(), values.end()),
                &ValuesCastHelper<ValueType, KeyValuePair, Values::KeyValuePair>::cast)), end_(
            boost::make_transform_iterator(
                boost::make_filter_iterator(filter, values.end(), values.end()),
                &ValuesCastHelper<ValueType, KeyValuePair, Values::KeyValuePair>::cast)), constBegin_(
            boost::make_transform_iterator(
                boost::make_filter_iterator(filter,
                    ((const Values&) values).begin(),
                    ((const Values&) values).end()),
                &ValuesCastHelper<const ValueType, ConstKeyValuePair,
                    Values::ConstKeyValuePair>::cast)), constEnd_(
            boost::make_transform_iterator(
                boost::make_filter_iterator(filter,
                    ((const Values&) values).end(),
                    ((const Values&) values).end()),
                &ValuesCastHelper<const ValueType, ConstKeyValuePair,
                    Values::ConstKeyValuePair>::cast)) {
    }

    friend class Values;
    iterator begin_;
    iterator end_;
    const_const_iterator constBegin_;
    const_const_iterator constEnd_;
  };

  /* ************************************************************************* */
  template<class ValueType>
  class Values::ConstFiltered {
  public:
    /** A const key-value pair, with the value a specific derived Value type. */
    typedef _ValuesConstKeyValuePair<ValueType> KeyValuePair;
    typedef KeyValuePair value_type;

    typedef typename Filtered<ValueType>::const_const_iterator iterator;
    typedef typename Filtered<ValueType>::const_const_iterator const_iterator;

    /** Conversion from Filtered to ConstFiltered */
    ConstFiltered(const Filtered<ValueType>& rhs) :
      begin_(rhs.beginConst()),
      end_(rhs.endConst()) {}

    iterator begin() { return begin_; }
    iterator end() { return end_; }
    const_iterator begin() const { return begin_; }
    const_iterator end() const { return end_; }

    /** Returns the number of values in this view */
    size_t size() const {
      size_t i = 0;
      for (const_iterator it = begin(); it != end(); ++it)
        ++i;
      return i;
    }

    FastList<Key> keys() const {
      FastList<Key> result;
      for(const_iterator it = begin(); it != end(); ++it)
        result.push_back(it->key);
      return result;
    }

  private:
    friend class Values;
    const_iterator begin_;
    const_iterator end_;
    ConstFiltered(
        const boost::function<bool(const Values::ConstKeyValuePair&)>& filter,
        const Values& values) {
      // We remove the const from values to create a non-const Filtered
      // view, then pull the const_iterators out of it.
      const Filtered<ValueType> filtered(filter, const_cast<Values&>(values));
      begin_ = filtered.beginConst();
      end_ = filtered.endConst();
    }
  };

  /* ************************************************************************* */
  /** Constructor from a Filtered view copies out all values */
  template<class ValueType>
  Values::Values(const Values::Filtered<ValueType>& view) {
    BOOST_FOREACH(const typename Filtered<ValueType>::KeyValuePair& key_value, view) {
      Key key = key_value.key;
      insert(key, static_cast<const ValueType&>(key_value.value));
    }
  }

  /* ************************************************************************* */
  template<class ValueType>
  Values::Values(const Values::ConstFiltered<ValueType>& view) {
    BOOST_FOREACH(const typename ConstFiltered<ValueType>::KeyValuePair& key_value, view) {
      Key key = key_value.key;
      insert(key, static_cast<const ValueType&>(key_value.value));
    }
  }

  /* ************************************************************************* */
  Values::Filtered<Value>
  inline Values::filter(const boost::function<bool(Key)>& filterFcn) {
    return filter<Value>(filterFcn);
  }

  /* ************************************************************************* */
  template<class ValueType>
  Values::Filtered<ValueType>
  Values::filter(const boost::function<bool(Key)>& filterFcn) {
    return Filtered<ValueType>(boost::bind(&filterHelper<ValueType>, filterFcn, _1), *this);
  }

  /* ************************************************************************* */
  Values::ConstFiltered<Value>
  inline Values::filter(const boost::function<bool(Key)>& filterFcn) const {
    return filter<Value>(filterFcn);
  }

  /* ************************************************************************* */
  template<class ValueType>
  Values::ConstFiltered<ValueType>
  Values::filter(const boost::function<bool(Key)>& filterFcn) const {
    return ConstFiltered<ValueType>(boost::bind(&filterHelper<ValueType>, filterFcn, _1), *this);
  }

  /* ************************************************************************* */
   template<>
   inline bool Values::filterHelper<Value>(const boost::function<bool(Key)> filter,
       const ConstKeyValuePair& key_value) {
     // Filter and check the type
     return filter(key_value.key);
   }

   /* ************************************************************************* */

    namespace internal {

    // Check the type and throw exception if incorrect
    template<typename ValueType>
    struct handle {
      ValueType operator()(Key j, const gtsam::Value * const pointer) {
        try {
          // value returns a const ValueType&, and the return makes a copy !!!!!
          return dynamic_cast<const GenericValue<ValueType>&>(*pointer).value();
        } catch (std::bad_cast &) {
          throw ValuesIncorrectType(j, typeid(*pointer), typeid(ValueType));
        }
      }
    };

    // Handle dynamic vectors
    template<>
    struct handle<Eigen::Matrix<double, -1, 1> > {
       Eigen::Matrix<double, -1, 1> operator()(Key j,
          const gtsam::Value * const pointer) {
        try {
//<<<<<<< HEAD
          // value returns a const Vector&, and the return makes a copy !!!!!
          return dynamic_cast<const GenericValue<Eigen::Matrix<double, -1, 1> >&>(*pointer).value();
//=======
//          return reinterpret_cast<const GenericValue<Eigen::Matrix<double, -1, 1> >&>(*pointer).value();
//>>>>>>> feature/FixFixedValues
        } catch (std::bad_cast &) {
          // If a fixed vector was stored, we end up here as well.
          throw ValuesIncorrectType(j, typeid(*pointer),
              typeid(Eigen::Matrix<double, -1, 1>));
        }
      }
    };

    // Handle dynamic matrices
    template<int N>
    struct handle<Eigen::Matrix<double, -1, N> > {
       Eigen::Matrix<double, -1, N> operator()(Key j,
          const gtsam::Value * const pointer) {
        try {
          // value returns a const Matrix&, and the return makes a copy !!!!!
          return dynamic_cast<const GenericValue<Eigen::Matrix<double, -1, N> >&>(*pointer).value();
        } catch (std::bad_cast &) {
          // If a fixed matrix was stored, we end up here as well.
          throw ValuesIncorrectType(j, typeid(*pointer),
              typeid(Eigen::Matrix<double, -1, N>));
        }
      }
    };

    // Request for a fixed vector
    // TODO(jing): is this piece of code really needed ???
    template<int M>
    struct handle<Eigen::Matrix<double, M, 1> > {
      Eigen::Matrix<double, M, 1> operator()(Key j,
          const gtsam::Value * const pointer) {
        try {
          // value returns a const VectorM&, and the return makes a copy !!!!!
          return dynamic_cast<const GenericValue<Eigen::Matrix<double, M, 1> >&>(*pointer).value();
        } catch (std::bad_cast &) {
          // Check if a dynamic vector was stored
          Matrix A = handle<Eigen::VectorXd>()(j, pointer); // will throw if not....
          // Yes: check size, and throw if not a match
          if (A.rows() != M || A.cols() != 1)
            throw NoMatchFoundForFixed(M, 1, A.rows(), A.cols());
          else
            // This is not a copy because of RVO
            return A;
        }
      }
    };

    // Request for a fixed matrix
    template<int M, int N>
    struct handle<Eigen::Matrix<double, M, N> > {
      Eigen::Matrix<double, M, N> operator()(Key j,
          const gtsam::Value * const pointer) {
        try {
          // value returns a const MatrixMN&, and the return makes a copy !!!!!
          return dynamic_cast<const GenericValue<Eigen::Matrix<double, M, N> >&>(*pointer).value();
        } catch (std::bad_cast &) {
          // Check if a dynamic matrix was stored
          Matrix A = handle<Eigen::MatrixXd>()(j, pointer); // will throw if not....
          // Yes: check size, and throw if not a match
          if (A.rows() != M || A.cols() != N)
            throw NoMatchFoundForFixed(M, N, A.rows(), A.cols());
          else
            // This is not a copy because of RVO
            return A;
        }
      }
    };


    } // internal

   /* ************************************************************************* */
   template<typename ValueType>
   ValueType Values::at(Key j) const {
     // Find the item
     KeyValueMap::const_iterator item = values_.find(j);

     // Throw exception if it does not exist
     if(item == values_.end())
       throw ValuesKeyDoesNotExist("at", j);

    // Check the type and throw exception if incorrect
    const Value& value = *item->second;
    try {
      return dynamic_cast<const GenericValue<ValueType>&>(value).value();
    } catch (std::bad_cast &) {
      // NOTE(abe): clang warns about potential side effects if done in typeid
      const Value* value = item->second;
      throw ValuesIncorrectType(j, typeid(*value), typeid(ValueType));
    }
  }

  /* ************************************************************************* */
  template<typename ValueType>
  boost::optional<const ValueType&> Values::exists(Key j) const {
    // Find the item
    KeyValueMap::const_iterator item = values_.find(j);

    if(item != values_.end()) {
      // dynamic cast the type and throw exception if incorrect
      const Value& value = *item->second;
      try {
        return dynamic_cast<const GenericValue<ValueType>&>(value).value();
      } catch (std::bad_cast &) {
        // NOTE(abe): clang warns about potential side effects if done in typeid
        const Value* value = item->second;
        throw ValuesIncorrectType(j, typeid(*value), typeid(ValueType));
      }
     } else {
      return boost::none;
    }
  }

  /* ************************************************************************* */

  // wrap all fixed in dynamics when insert and update
  namespace internal {

  // general type
  template<typename ValueType>
  struct handle_wrap {
    inline gtsam::GenericValue<ValueType> operator()(Key j, const ValueType& val) {
      return gtsam::GenericValue<ValueType>(val);
    }
  };

  // when insert/update a fixed size vector: convert to dynamic size
  template<int M>
  struct handle_wrap<Eigen::Matrix<double, M, 1> > {
    inline gtsam::GenericValue<Eigen::Matrix<double, -1, 1> > operator()(
        Key j, const Eigen::Matrix<double, M, 1>& val) {
      return gtsam::GenericValue<Eigen::Matrix<double, -1, 1> >(val);
    }
  };

  // when insert/update a fixed size matrix: convert to dynamic size
  template<int M, int N>
  struct handle_wrap<Eigen::Matrix<double, M, N> > {
    inline gtsam::GenericValue<Eigen::Matrix<double, -1, -1> > operator()(
        Key j, const Eigen::Matrix<double, M, N>& val) {
      return gtsam::GenericValue<Eigen::Matrix<double, -1, -1> >(val);
    }
  };

  }

  // insert a templated value
  template<typename ValueType>
  void Values::insert(Key j, const ValueType& val) {
    insert(j, static_cast<const Value&>(internal::handle_wrap<ValueType>()(j, val)));
  }

  // update with templated value
  template <typename ValueType>
  void Values::update(Key j, const ValueType& val) {
    update(j, static_cast<const Value&>(internal::handle_wrap<ValueType>()(j, val)));
  }

}
