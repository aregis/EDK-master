/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONFIG_OBJECTBUILDER_H_
#define HUESTREAM_CONFIG_OBJECTBUILDER_H_

#include "huestream/common/serialize/ObjectBuilderBase.h"
#include "huestream/common/data/BridgeSettings.h"
#include "huestream/common/time/ITimeProvider.h"

#include <memory>
#include <string>

namespace huestream {

class ObjectBuilder : public ObjectBuilderBase {
 protected:
 public:
    explicit ObjectBuilder(std::shared_ptr<BridgeSettings> _bridgeSettings = nullptr);
 protected:
    std::shared_ptr<Serializable> ConstructInstanceOf(std::string type) override;
    BridgeSettingsPtr _bridgeSettings;
};

}  // namespace huestream

#endif  // HUESTREAM_CONFIG_OBJECTBUILDER_H_
