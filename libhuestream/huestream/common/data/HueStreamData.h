/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_HUESTREAMDATA_H_
#define HUESTREAM_COMMON_DATA_HUESTREAMDATA_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/common/data/Bridge.h"

#include <map>
#include <string>
#include <sstream>

namespace huestream {


class HueStreamData : public Serializable {
 public:
    static constexpr const char* type = "huestream.HueStreamData";

    explicit HueStreamData(BridgeSettingsPtr bridgeSettings);

    virtual ~HueStreamData() {}

    /**
     empty the data object
     */
    void Clear();

    /**
     get the currently active bridge
     @return currently selected bridge
     */
    BridgePtr GetActiveBridge() const;

    /**
     set the currently active bridge
     */
    void SetActiveBridge(BridgePtr bridge);

    /**
     clear the currently active bridge
     */
    void ClearActiveBridge();

    /**
     remove a bridge
     @return whether the bridge was in the list and thus removed
     */
    bool RemoveBridge(BridgePtr bridge);

    /**
     change active bridge to most recently used known valid bridge from the provided list
     @return whether a known valid bridge was found in the list
     */
    bool RediscoverKnownBridge(BridgeListPtr discoveredBridges);

    /**
     @return list of all known bridges that are authenticated for streaming
     */
    BridgeListPtr GetAllKnownBridges();

    void Serialize(JSONNode *node) const override;

    void Deserialize(JSONNode *node) override;

    std::string GetTypeName() const override;

    PROP_DEFINE(HueStreamData, int, version, Version);
    PROP_DEFINE(HueStreamData, BridgeListPtr, bridges, Bridges);

protected:
    BridgeSettingsPtr _bridgeSettings;
};

/**
 shared pointer to a huestream::HueStreamData object
 */
SMART_POINTER_TYPES_FOR(HueStreamData)

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_HUESTREAMDATA_H_
