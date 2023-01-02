/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/serialize/ObjectBuilderBase.h>
#include <huestream/common/serialize/Serializable.h>

#include <memory>

namespace huestream {

    std::shared_ptr<Serializable> ObjectBuilderBase::Build(JSONNode *node) {
        if (!SerializerHelper::IsAttributeSet(node, Serializable::AttributeType)) {
            return nullptr;
        }
        auto type = Serializable::GetTypeFromJson(node);
        std::shared_ptr<Serializable> obj = ConstructInstanceOf(type);
        return obj;
    }
}  // namespace huestream
