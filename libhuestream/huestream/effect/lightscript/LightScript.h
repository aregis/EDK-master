/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_LIGHTSCRIPT_LIGHTSCRIPT_H_
#define HUESTREAM_EFFECT_LIGHTSCRIPT_LIGHTSCRIPT_H_

#include "huestream/common/serialize/Serializable.h"
#include "huestream/effect/effects/base/Effect.h"
#include "huestream/effect/lightscript/Action.h"
#include "huestream/common/data/Location.h"
#include "huestream/effect/lightscript/Timeline.h"

#include <stdarg.h>

#include <string>
#include <vector>

namespace huestream {

    /**
     collection of huestream::Action s (which are light effects with a certion time position and layer) and metadata which can be played if bound to a huestream::ITimeline
     */
    class LightScript : public Serializable {
    public:
        LightScript();

        LightScript(std::string name, int64_t length);

        LightScript(std::string name, int64_t length, LocationListPtr idealSetup);

        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        /**
         add an action to the light script
         */
        void AddAction(ActionPtr newAction);

        /**
         bind this lightscript to a timeline such that it will play the actions according to the time position of the timeline
         */
        void BindTimeline(TimelinePtr timeline);

        /**
         finishing the light script finishes all the underlying actions such that they are cleaned up from the engine
         */
        void Finish();

        /**
         get the previously bound timeline
         */
        TimelinePtr GetTimeline();

        static HUESTREAM_EXPORT std::string type;
        static HUESTREAM_EXPORT std::string AttributeVersionMajor;
        static HUESTREAM_EXPORT std::string AttributeVersionMinor;
        static HUESTREAM_EXPORT int majorVersion;
        static HUESTREAM_EXPORT int minorVersion;

    /**
     set user friendly name of the light script
     */
    PROP_DEFINE(LightScript, std::string, name, Name);

    /**
     set light locations which would produce most ideal result for this lightscript
     */
    PROP_DEFINE(LightScript, LocationListPtr, idealSetup, IdealSetup);

    /**
     set the length of this light script
     */
    PROP_DEFINE(LightScript, int64_t, length, Length);

    /**
     overwrite the whole list of actions in this light script
     */
    PROP_DEFINE(LightScript, ActionListPtr, actions, Actions);

    private:
        void SerializeLayers(JSONNode *node) const;

        void DeserializeLayers(JSONNode *node);

        int FindActionIndex(const ActionPtr &newAction) const;

        void UpdateActions();

        TimelinePtr _timeline;
    };

    /**
     shared pointer to a huestream::LightScript object
     */
    SMART_POINTER_TYPES_FOR(LightScript);

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_LIGHTSCRIPT_LIGHTSCRIPT_H_
