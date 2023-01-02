/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_LIGHT_H_
#define HUESTREAM_COMMON_DATA_LIGHT_H_

#include "huestream/common/data/Color.h"
#include "huestream/common/data/Location.h"
#include "huestream/common/serialize/SerializerHelper.h"

#include <memory>
#include <vector>
#include <string>

namespace huestream {

    /**
     defintion of a lightpoint which resides at a certain location and can be set to a certain color
     */
    class Light : public Serializable {
    public:
        static constexpr const char* type = "huestream.Light";

    /**
     Set id of light
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Light, std::string, id, Id);

    /**
     Set id v1 of light
     @note mostly internal use, imported from bridge
    */
    PROP_DEFINE(Light, std::string, idV1, IdV1);

    /**
     Set name of light
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Light, std::string, name, Name);

    /**
     Set model id of light
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Light, std::string, model, Model);

    /**
     Set color of light
     */
    PROP_DEFINE(Light, Color, color, Color);

    /**
     Set location of light
     @note internal use, imported from bridge: location x and y coordinates range between -1 and 1
     */
    PROP_DEFINE(Light, Location, position, Position);

    /**
     Set reachability state of light
     */
    PROP_DEFINE_BOOL(Light, bool, reachable, Reachable);

    /**
     Set on state of light
     @TODO remove when group on state is available in clipv2
    */
    PROP_DEFINE_BOOL(Light, bool, on, On);

    /**
     Set brightness of light
     @TODO remove when group brightness is available in clipv2
    */
    PROP_DEFINE(Light, double, brightness, Brightness);

    /**
     Set archetype of light
    */
    PROP_DEFINE(Light, std::string, archetype, Archetype);

    /**
     Indicates whether or not the light support dynamic features.
    */
    PROP_DEFINE_BOOL(Light, bool, dynamic, Dynamic);

    /**
     Indicates whether or not the light dynamic features are enabled.
    */
    PROP_DEFINE_BOOL(Light, bool, dynamicEnabled, DynamicEnabled);



    public:
        Light();

        /**
         constructor
         @param id Light identifier
         @param position Light location
         @param name Light friendly name
         @param model Light model id
         @param reachable Light reachability state
         */
        Light(std::string id, Location position, std::string name = "", std::string model = "", std::string archetype = "", bool reachable = true, bool on = false);

        /**
         destructor
         */
        virtual ~Light();

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;

        Light* Clone();
    };

    /**
     shared pointer to a huestream::Light object
     */
    SMART_POINTER_TYPES_FOR(Light);

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_LIGHT_H_
