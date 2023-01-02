/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_SCENE_H_
#define HUESTREAM_COMMON_DATA_SCENE_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/common/serialize/Serializable.h"
#include "huestream/common/data/Color.h"
#include "huestream/common/data/Light.h"

#include <memory>
#include <vector>
#include <string>
#include <map>


namespace huestream {
    /**
     defintion of a light scene
     */
    class Scene : public Serializable {
    public:
        static constexpr const char* type = "huestream.Scene";

    /**
     Get id of scene
    */
    PROP_DEFINE(Scene, std::string, id, Id);

    /**
     Get tag of scene
     */
    PROP_DEFINE(Scene, std::string, tag, Tag);

    /**
     Get name of scene
     */
    PROP_DEFINE(Scene, std::string, name, Name);

    /**
     Get tag of scene
     */
    PROP_DEFINE(Scene, std::string, appData, AppData);

    /**
     Get image associated with scene
    */
    PROP_DEFINE(Scene, std::string, image, Image);

    /**
    Get list of lights associated with the scene
    */
    PROP_DEFINE(Scene, LightListPtr, lights, Lights);

    /*
    Get the color palette for this scene
    */
    PROP_DEFINE_BOOL(Scene, bool, dynamic, Dynamic);


    public:
        Scene();

        /**
         constructor
         @param id Scene id
         @param tag Scene tag
         @param lightsColor Map of light id and color
         @param name Scene friendly name
         @param appData Scene app data field
         @param image Scene associated image
         */
        explicit Scene(const std::string& id, const std::string& tag, const std::string& name = "", const std::string& appData = "", const std::string& image = "");

        /**
         destructor
         */
        virtual ~Scene();

        std::string GetGroupId() const;

        int GetDefaultSceneId() const;

        bool AllLightsAreDynamic() const;

        bool AtLeastOneLightIsDynamic() const;

        Scene* Clone();

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;
    };

    /**
     shared pointer to a huestream::Scene object
     */
    SMART_POINTER_TYPES_FOR(Scene);

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_SCENE_H_
