/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Light.h>
#include <huestream/common/serialize/Serializable.h>
#include <huestream/config/ObjectBuilder.h>

#include <string>
#include <memory>

namespace huestream {

    void
    Serializable::SetMemberIfAttributeExists(const JSONNode *node, const std::string &attributeName, Serializable* member) {
        if (SerializerHelper::IsAttributeSet(node, attributeName)) {
            JSONNode v = (*node)[attributeName];
            member->Deserialize(&v);
        }
    }

    void Serializable::SerializeMember(JSONNode *node, const std::string &attributeName, const Serializable &obj) {
        JSONNode v;
        obj.Serialize(&v);
        v.set_name(attributeName);
        node->push_back(v);
    }

    std::shared_ptr<ObjectBuilderBase> Serializable::object_builder_{new ObjectBuilder{nullptr}};

    std::string Serializable::AttributeType = "type";

    void Serializable::Serialize(JSONNode *node) const {
        node->push_back(JSONNode(AttributeType, GetTypeName()));
    }

    void Serializable::Deserialize(JSONNode* /*node*/) {
    }

    std::string Serializable::GetTypeFromJson(const JSONNode *node) {
        return SerializerHelper::GetAttributeValue(node, AttributeType).as_string();
    }

    bool Serializable::IsOfType(const JSONNode *node, const std::string &type) {
        return SerializerHelper::IsAttributeSet(node, type) && GetTypeFromJson(node) == type;
    }

    std::shared_ptr<Serializable> Serializable::DeserializeFromJson(JSONNode *node) {
        std::shared_ptr<Serializable> obj;

        const auto object_builder = object_builder_;
        if (object_builder) {
            obj = object_builder->Build(node);
            if (obj != nullptr)
                obj->Deserialize(node);
        }

        return obj;
    }

    void Serializable::SetObjectBuilder(std::shared_ptr<ObjectBuilderBase> builder) {
        object_builder_ = builder;
    }

    void Serializable::DeserializeValue(const JSONNode *node,
                                        const std::string &attribute_name,
                                        double *value,
                                        double default_value) {
        if (SerializerHelper::IsAttributeSet(node, attribute_name)) {
            *value = (*node)[attribute_name].as_float();
        } else {
            *value = default_value;
        }
    }

    void Serializable::DeserializeValue(const JSONNode *node,
                                        const std::string &attribute_name,
                                        int *value,
                                        int default_value) {
        if (SerializerHelper::IsAttributeSet(node, attribute_name)) {
            *value = static_cast<int>((*node)[attribute_name].as_int());
        } else {
            *value = default_value;
        }
    }

    void Serializable::DeserializeValue(const JSONNode *node,
                                        const std::string &attribute_name,
                                        int64_t *value,
                                        int64_t  default_value) {
        if (SerializerHelper::IsAttributeSet(node, attribute_name)) {
            *value = static_cast<int>((*node)[attribute_name].as_int());
        } else {
            *value = default_value;
        }
    }

    void Serializable::DeserializeValue(const JSONNode *node,
                                        const std::string &attribute_name,
                                        bool *value,
                                        bool default_value) {
        if (SerializerHelper::IsAttributeSet(node, attribute_name)) {
            *value = (*node)[attribute_name].as_bool();
        } else {
            *value = default_value;
        }
    }

    void Serializable::DeserializeValue(const JSONNode *node,
                                        const std::string &attribute_name,
                                        std::string *value,
                                        const std::string default_value) {
        if (SerializerHelper::IsAttributeSet(node, attribute_name)) {
            *value = (*node)[attribute_name].as_string();
        } else {
            *value = default_value;
        }
    }

    std::string Serializable::SerializeText() const {
        JSONNode n;
        Serialize(&n);
        return n.write_formatted();
    }

    void Serializable::DeserializeText(const std::string &jsonString) {
        JSONNode n = libjson::parse(jsonString);
        Deserialize(&n);
    }


    std::shared_ptr<Serializable> Serializable::DeserializeFromJsonText(const std::string &jsonString) {
        JSONNode n = libjson::parse(jsonString);
        return DeserializeFromJson(&n);
    }


}  // namespace huestream
