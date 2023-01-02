/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_SERIALIZE_OBJECTBUILDERBASE_H_
#define HUESTREAM_COMMON_SERIALIZE_OBJECTBUILDERBASE_H_

#include "libjson/libjson.h"

#include <memory>
#include <string>

namespace huestream {

    class Serializable;

    class ObjectBuilderBase {
    public:
        virtual ~ObjectBuilderBase() = default;

        std::shared_ptr<Serializable> Build(JSONNode *node);

    protected:
        virtual std::shared_ptr<Serializable> ConstructInstanceOf(std::string type) = 0;
    };
}  // namespace huestream

#endif  // HUESTREAM_COMMON_SERIALIZE_OBJECTBUILDERBASE_H_
