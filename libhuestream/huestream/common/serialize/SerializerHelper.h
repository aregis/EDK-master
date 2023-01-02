/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_SERIALIZE_SERIALIZERHELPER_H_
#define HUESTREAM_COMMON_SERIALIZE_SERIALIZERHELPER_H_

#include "libjson/libjson.h"

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <type_traits>

namespace huestream {

#if defined(USE_EXPORT)
#include <huestream/common/serialize/huestream_export.h>
#else
#define HUESTREAM_EXPORT
#endif

#define NEW_PTR(type, ...) std::shared_ptr<type>(new type(__VA_ARGS__))
#define NEW_LIST_PTR(type, ...) std::shared_ptr<type##List>(new type##List())

#define SMART_POINTER_TYPES_FOR(type) \
typedef std::shared_ptr<type> type##Ptr; \
typedef std::vector<type##Ptr> type##List; \
typedef std::shared_ptr<type##List> type##ListPtr;

#define PROP_DEFINE_VIRTUAL(className, type, privateName, publicName) \
public: \
  virtual const type &Get##publicName() const = 0; \
  virtual void Set##publicName(const type &privateName) = 0;

#define PROP_DEFINE_OVERRIDE(className, type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const override; \
  void Set##publicName(const type &privateName) override; \
protected: \
  type _##privateName;

template<class T>
std::shared_ptr<std::vector<std::shared_ptr<T>>> clone_list(std::shared_ptr<std::vector<std::shared_ptr<T>>> input) {
    auto output = std::make_shared<std::vector<std::shared_ptr<T>>>();
    for (auto item : *input) {
        output->emplace_back(std::shared_ptr<T>(item->Clone()));
    };
    return output;
};

#define PROP_DEFINE(className, type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const; \
  void Set##publicName(const type &privateName); \
protected: \
  type _##privateName;


#define PROP_DEFINE_GET(className, type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const; \
protected: \
  type _##privateName;

#define PROP_GET(type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const{\
    return _##privateName;\
  } \
protected: \
  type _##privateName;

#define PROP_SET(type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  void Set##publicName(const type &privateName) {\
    _##privateName = privateName;\
  } \
protected: \
  type _##privateName;

#define PROP_GETSET(type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const{\
    return _##privateName;\
  } \
  void Set##publicName(const type &privateName) {\
    _##privateName = privateName;\
  } \
protected: \
  type _##privateName;

#define PROP_GETSET_MODIFIED(type, privateName, publicName, modified) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const{\
    return _##privateName;\
  } \
  void Set##publicName(const type &privateName) {\
    _##privateName = privateName;\
    modified = true;\
  } \
protected: \
  type _##privateName;

#define PROP_DEFINE_IMPL(className, type, privateName, publicName) PROP_GETSET(type, privateName, publicName)

#define PROP_DEFINE_IMPL_GET(className, type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const { return _##privateName;} \
protected: \
  type _##privateName;


#define PROP_DEFINE_IMPL_ON_UPDATE_CALL(className, type, privateName, publicName, updateCallName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const type &Get##publicName() const { return _##privateName;} \
  void Set##publicName(const type &privateName) { _##privateName = privateName; updateCallName();} \
protected: \
  type _##privateName;

#define PROP_IMPL(className, type, privateName, publicName) \
  std::string className::Attribute##publicName = ""#publicName""; \
  const type &className::Get##publicName() const { return _##privateName;} \
  void className::Set##publicName(const type &privateName) { _##privateName = privateName;}

#define PROP_IMPL_GET(className, type, privateName, publicName) \
  std::string className::Attribute##publicName = ""#publicName""; \
  const type &className::Get##publicName() const { return _##privateName;}



#define PROP_IMPL_ON_UPDATE_CALL(className, type, privateName, publicName, updateCallName) \
  std::string className::Attribute##publicName = ""#publicName""; \
  const type &className::Get##publicName() const { return _##privateName;} \
  void className::Set##publicName(const type &privateName) { _##privateName = privateName; updateCallName();}

#define PROP_DEFINE_BOOL(className, type, privateName, publicName) \
public: \
  static HUESTREAM_EXPORT std::string Attribute##publicName;\
  const bool& publicName() const; \
  void Set##publicName(const bool &privateName); \
protected: \
  bool _##privateName;

#define PROP_IMPL_BOOL(className, type, privateName, publicName) \
  std::string className::Attribute##publicName = ""#publicName""; \
  const bool &className::publicName() const { return _##privateName;} \
  void className::Set##publicName(const bool &privateName) { _##privateName = privateName;} \

#define AttributeTypeName "type"
class SerializerHelper {
 public:
    static JSONNode GetAttributeValue(const JSONNode *node, const std::string &attribute) {
        auto i = node->find(attribute);
        if (i != node->end())
            return *i;
        return JSONNode();
    }

    static bool IsAttributeSet(const JSONNode *node, const std::string &attributeName) {
        auto i = node->find(attributeName);
        return (i != node->end());
    }
    static std::string GetTypeFromJson(const JSONNode *node) {
        return GetAttributeValue(node, AttributeTypeName).as_string();
    }
    static bool IsOfType(const JSONNode *node, const std::string &type) {
        return IsAttributeSet(node, type) && GetTypeFromJson(node) == type;
    }
};

}  // namespace huestream

#endif  // HUESTREAM_COMMON_SERIALIZE_SERIALIZERHELPER_H_
