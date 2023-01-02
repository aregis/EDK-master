/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_CHANNEL_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_CHANNEL_H_

#include "huestream/common/serialize/Serializable.h"
#include "huestream/common/data/Location.h"
#include "huestream/effect/animation/animations/base/Animation.h"

#include <stdarg.h>

#include <string>
#include <vector>

namespace huestream {

    /**
     light channel which defines a color animation meant to be displayed at a certain location (compare to audio channels e.g. left/right for stereo)
     */
    class Channel : public Serializable {
    public:
        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        static constexpr const char* type = "huestream.Channel";

    PROP_DEFINE(Channel, Location, position, Position);
    PROP_DEFINE(Channel, AnimationPtr, r, R);
    PROP_DEFINE(Channel, AnimationPtr, g, G);
    PROP_DEFINE(Channel, AnimationPtr, b, B);
    PROP_DEFINE(Channel, AnimationPtr, a, A);

    public:
        Channel(Location pos, AnimationPtr r, AnimationPtr g, AnimationPtr b, AnimationPtr a);

        Channel();
    };

    /**
     shared pointer to a huestream::Channel object
     */
    SMART_POINTER_TYPES_FOR(Channel);

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_CHANNEL_H_
