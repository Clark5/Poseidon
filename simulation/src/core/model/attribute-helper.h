/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef ATTRIBUTE_HELPER_H
#define ATTRIBUTE_HELPER_H

#include "attribute.h"
#include "attribute-accessor-helper.h"
#include <sstream>
#include "fatal-error.h"

namespace ns3 {

/**
 * \ingroup attribute
 * \defgroup attributehelper Attribute Helper
 *
 * All these macros can be used to generate automatically the code
 * for subclasses of AttributeValue, AttributeAccessor, and, AttributeChecker,
 * which can be used to give attribute powers to a normal class. i.e.,
 * the user class can then effectively be made an attribute.
 *
 * There are two kinds of helper macros:
 *  1) The simple macros.
 *    - ATTRIBUTE_HELPER_HEADER(type)
 *    - ATTRIBUTE_HELPER_CPP
 *  2) The more complex macros.
 *
 * The simple macros are implemented in terms of the complex
 * macros and should generally be preferred over the complex macros.
 */
  
/**
 * \ingroup attributehelper
 *
 * A simple string-based attribute checker
 *
 * \param name value type of the attribute
 * \param underlying underlying type name
 * \return Ptr to AttributeChecker
 */
template <typename T, typename BASE>
Ptr<AttributeChecker>
MakeSimpleAttributeChecker (std::string name, std::string underlying)
{
  struct SimpleAttributeChecker : public BASE
  {
    virtual bool Check (const AttributeValue &value) const {
      return dynamic_cast<const T *> (&value) != 0;
    }
    virtual std::string GetValueTypeName (void) const {
      return m_type;
    }
    virtual bool HasUnderlyingTypeInformation (void) const {
      return true;
    }
    virtual std::string GetUnderlyingTypeInformation (void) const {
      return m_underlying;
    }
    virtual Ptr<AttributeValue> Create (void) const {
      return ns3::Create<T> ();
    }
    virtual bool Copy (const AttributeValue &source, AttributeValue &destination) const {
      const T *src = dynamic_cast<const T *> (&source);
      T *dst = dynamic_cast<T *> (&destination);
      if (src == 0 || dst == 0)
        {
          return false;
        }
      *dst = *src;
      return true;
    }
    std::string m_type;
    std::string m_underlying;
  } *checker = new SimpleAttributeChecker ();
  checker->m_type = name;
  checker->m_underlying = underlying;
  return Ptr<AttributeChecker> (checker, false);
}

}

/**
 * \ingroup attributehelper
 * \param type the name of the class
 *
 * This macro defines and generates the code for the implementation 
 * of the \c Make<type>Accessor template functions. This macro is typically
 * invoked in a class header to allow users of this class to view and
 * use the template functions defined here. This macro is implemented
 * through the helper templates functions ns3::MakeAccessorHelper<>.
 */
#define ATTRIBUTE_ACCESSOR_DEFINE(type)                                 \
  template <typename T1>                                                \
  Ptr<const AttributeAccessor> Make ## type ## Accessor (T1 a1)             \
  {                                                                     \
    return MakeAccessorHelper<type ## Value> (a1);                        \
  }                                                                     \
  template <typename T1, typename T2>                                   \
  Ptr<const AttributeAccessor> Make ## type ## Accessor (T1 a1, T2 a2)      \
  {                                                                     \
    return MakeAccessorHelper<type ## Value> (a1, a2);                    \
  }

/**
 * \ingroup attributehelper
 * \internal
 */
#define ATTRIBUTE_VALUE_DEFINE_WITH_NAME(type,name)                     \
  class name ## Value : public AttributeValue                             \
  {                                                                     \
public:                                                               \
    name ## Value ();                                                     \
    name ## Value (const type &value);                                    \
    void Set (const type &value);                                       \
    type Get (void) const;                                              \
    template <typename T>                                               \
    bool GetAccessor (T &value) const {                                 \
      value = T (m_value);                                              \
      return true;                                                      \
    }                                                                   \
    virtual Ptr<AttributeValue> Copy (void) const;                      \
    virtual std::string SerializeToString (Ptr<const AttributeChecker> checker) const; \
    virtual bool DeserializeFromString (std::string value, Ptr<const AttributeChecker> checker); \
private:                                                              \
    type m_value;                                                       \
  };


/**
 * \ingroup attributehelper
 * \param type the name of the class.
 *
 * This macro defines the class \c typeValue associated to class \c type.
 * This macro is typically invoked in the class header file.
 */
#define ATTRIBUTE_VALUE_DEFINE(type)                                    \
  ATTRIBUTE_VALUE_DEFINE_WITH_NAME (type,type)


/**
 * \ingroup attributehelper
 * \param type the name of the class
 *
 * This macro defines the conversion operators for class \c type to and
 * from instances of type Attribute.
 * Typically invoked in the class header file.
 */
#define ATTRIBUTE_CONVERTER_DEFINE(type)

/**
 * \ingroup attributehelper
 * \param type the name of the class
 *
 * This macro defines the \c typeChecker class and the associated
 * \c Make<type>Checker function.
 * Typically invoked in the class header file..
 */
#define ATTRIBUTE_CHECKER_DEFINE(type)                          \
  class type ## Checker : public AttributeChecker {};             \
  Ptr<const AttributeChecker> Make ## type ## Checker (void);       \


/**
 * \ingroup attributehelper
 * \internal
 */
#define ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(type,name)                  \
  name ## Value::name ## Value ()                                           \
    : m_value () {}                                                     \
  name ## Value::name ## Value (const type &value)                          \
    : m_value (value) {}                                                  \
  void name ## Value::Set (const type &v) {                               \
    m_value = v;                                                        \
  }                                                                     \
  type name ## Value::Get (void) const {                                  \
    return m_value;                                                     \
  }                                                                     \
  Ptr<AttributeValue>                                                   \
  name ## Value::Copy (void) const {                                      \
    return ns3::Create<name ## Value> (*this);                            \
  }                                                                     \
  std::string                                                           \
    name ## Value::SerializeToString (Ptr<const AttributeChecker> checker) const { \
    std::ostringstream oss;                                             \
    oss << m_value;                                                     \
    return oss.str ();                                                  \
  }                                                                     \
  bool                                                                  \
    name ## Value::DeserializeFromString (std::string value, Ptr<const AttributeChecker> checker) { \
    std::istringstream iss;                                             \
    iss.str (value);                                                    \
    iss >> m_value;                                                     \
    return !iss.bad () && !iss.fail ();                                 \
  }

/**
 * \ingroup attributehelper
 * \param type the name of the class.
 *
 * This macro implements the \c typeValue class (including the 
 * \c typeValue::SerializeToString and \c typeValue::DeserializeFromString 
 * methods).
 * Typically invoked in the source file.
 */
#define ATTRIBUTE_VALUE_IMPLEMENT(type)                                 \
  ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME (type,type)


/**
 * \ingroup attributehelper
 * \param type the name of the class
 *
 * This macro implements the \c Make<type>Checker function.
 * Typically invoked in the source file..
 */
#define ATTRIBUTE_CHECKER_IMPLEMENT(type)                               \
  Ptr<const AttributeChecker> Make ## type ## Checker (void)                \
  {                                                                     \
    return MakeSimpleAttributeChecker<type ## Value,type ## Checker> (# type "Value", # type); \
  }                                                                     \

/**
 * \ingroup attributehelper
 * \internal
 */
#define ATTRIBUTE_CHECKER_IMPLEMENT_WITH_NAME(type,name)                    \
  Ptr<const AttributeChecker> Make ## type ## Checker (void)                \
  {                                                                     \
    return MakeSimpleAttributeChecker<type ## Value,type ## Checker> (# type "Value", name); \
  }                                                                     \

/**
 * \ingroup attributehelper
 * \param type the name of the class
 *
 * This macro should be invoked outside of the class
 * declaration in its public header.
 */
#define ATTRIBUTE_HELPER_HEADER(type)                                   \
  ATTRIBUTE_VALUE_DEFINE (type);                                        \
  ATTRIBUTE_ACCESSOR_DEFINE (type);                                     \
  ATTRIBUTE_CHECKER_DEFINE (type);

/**
 * \ingroup attributehelper
 * \param type the name of the class
 *
 * This macro should be invoked from the class implementation file.
 */
#define ATTRIBUTE_HELPER_CPP(type)                                      \
  ATTRIBUTE_CHECKER_IMPLEMENT (type);                                   \
  ATTRIBUTE_VALUE_IMPLEMENT (type);


#endif /* ATTRIBUTE_HELPER_H */
