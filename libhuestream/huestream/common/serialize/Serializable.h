/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_SERIALIZE_SERIALIZABLE_H_
#define HUESTREAM_COMMON_SERIALIZE_SERIALIZABLE_H_

#include "libjson/libjson.h"
#include "huestream/common/serialize/ObjectBuilderBase.h"
#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/effect/animation/data/TweenType.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace huestream {

    /**
     abstract base class for an object which can be serialized
     */
    class Serializable {
    private:
        static std::shared_ptr<ObjectBuilderBase> object_builder_;

    public:
        virtual ~Serializable() {
        }

        static HUESTREAM_EXPORT std::string AttributeType;

        static void SetObjectBuilder(std::shared_ptr<ObjectBuilderBase> builder);

        static std::shared_ptr<Serializable> DeserializeFromJson(JSONNode *node);

        static std::shared_ptr<Serializable> DeserializeFromJsonText(const std::string &jsonString);

        /**
         serialize this object into a JSON string
         @return serialized object as JSON string
         */
        virtual std::string SerializeText() const;

        /**
         deserialize this object from a JSON string
         @param jsonString serialized object as JSON string
         */
        virtual void DeserializeText(const std::string &jsonString);

        /**
         serializable objects override this method to implement which attributes are serialized
         @param[out] node Reference to JSONNode in which the serialized object is placed
         */
        virtual void Serialize(JSONNode *node) const;

        /**
         serializable objects override this method to implement which attributes are deserialized
         @param[in] node Reference to JSONNode from which to deserialize the object
         */
        virtual void Deserialize(JSONNode *node);

        /**
         serializable objects override this method to assign a name to their object type for serialization
         @return type name used for serialization
         */
        virtual std::string GetTypeName() const = 0;

        static std::string GetTypeFromJson(const JSONNode *node);

        static bool IsOfType(const JSONNode *node, const std::string &type);

        static void DeserializeValue(const JSONNode *node,
                                     const std::string &attribute_name,
                                     double *value,
                                     double default_value);

        static void DeserializeValue(const JSONNode *node,
                                     const std::string &attribute_name,
                                     int *value,
                                     int default_value);

        static void DeserializeValue(const JSONNode *node,
                                     const std::string &attribute_name,
                                     int64_t *value,
                                     int64_t default_value);

        static void DeserializeValue(const JSONNode *node,
                                     const std::string &attribute_name,
                                     std::string *value,
                                     const std::string default_value);

        static void DeserializeValue(const JSONNode *node,
                                     const std::string &attribute_name,
                                     bool *value,
                                     bool default_value);

        template<typename T>
        static void SerializeValue(JSONNode *node, const std::string &attribute_name, const T &value) {
            node->push_back(JSONNode(attribute_name, value));
        }

        template<typename T>
        static std::shared_ptr<T>
        DeserializeAttribute(JSONNode *node, const std::string &attribute_name, std::shared_ptr<T> default_value) {
            if (!SerializerHelper::IsAttributeSet(node, attribute_name))
                return default_value;

            JSONNode c = (*node)[attribute_name];
            auto value = std::static_pointer_cast<T>(DeserializeFromJson(&c));
            if (value == nullptr)
                return default_value;

            return value;
        }

        template<typename T>
        static void SerializeAttribute(JSONNode *node, const std::string &attribute_name, const T &value) {
            if (value == nullptr)
                return;

            JSONNode n;
            n.set_name(attribute_name);
            value->Serialize(&n);
            node->push_back(n);
        }

        static void SerializeMember(JSONNode *node, const std::string &attributeName, const Serializable &member);

        static void
        SetMemberIfAttributeExists(const JSONNode *node, const std::string &attributeName, Serializable *member);

        template<typename T, typename U>
        static void DeserializeList(const JSONNode *node, T* listPtr, const std::string &attributeName) {
            if (*listPtr == nullptr) {
                *listPtr = std::make_shared<std::vector<std::shared_ptr<U>>>();
            }
            listPtr->get()->clear();
            if (SerializerHelper::IsAttributeSet(node, attributeName)) {
                auto listNode = (*node)[attributeName];
                for (auto it = listNode.begin(); it != listNode.end(); ++it) {
                    JSONNode itemNode = *it;
                    auto o = std::static_pointer_cast<U>(DeserializeFromJson(&itemNode));
                    listPtr->get()->push_back(o);
                }
            }
        }

        template<typename T>
        static void SerializeList(JSONNode *node, const std::string &attributeName, const T &listPtr) {
            JSONNode arrayNode(JSON_ARRAY);
            for (auto i = 0; i < static_cast<int>(listPtr->size()); ++i) {
                JSONNode v;
                listPtr->at(i)->Serialize(&v);
                arrayNode.push_back(v);
            }
            arrayNode.set_name(attributeName);
            node->push_back(arrayNode);
        }
    };

    SMART_POINTER_TYPES_FOR(Serializable)
}  // namespace huestream

#endif  // HUESTREAM_COMMON_SERIALIZE_SERIALIZABLE_H_
