/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_ANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_ANIMATION_H_

#include "huestream/common/serialize/Serializable.h"
#include "huestream/effect/animation/data/TweenType.h"


#include <stdarg.h>

#include <memory>
#include <vector>
#include <limits>
#include <string>

namespace huestream {

#undef max
    const double INF = std::numeric_limits<double>::max();

    /**
     abstract base class for an animation which maps time to a value and can be used to animate properties of a light effect such as its intensity
     */
    class Animation : public Serializable {
    public:
    /**
     set marker for current position in milliseconds
     @note this is normally automatically done by an animation player
     */
    PROP_DEFINE(Animation, double, marker, Marker);

    /**
     manually set the value of this animation
     */
    PROP_DEFINE(Animation, double, value, Value);

    public:
        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        /**
         whenever the marker is changed, this is called to update the current value
         @param[out] value Reference to where new value should be stored
         @param[in] positionMs Current position in milliseconds for which to calculate the value
         */
        virtual void UpdateValue(double *value, double positionMs) = 0;

        /**
         get total duration in milliseconds of the animation
         */
        virtual double GetLengthMs() const = 0;

        Animation();

        virtual ~Animation() {}

        /**
         create an exact clone if this animation as a new object
         */
        virtual std::shared_ptr<Animation> Clone() = 0;

        /**
         respond to marker update
         @note mainly internal use
         */
        virtual void OnMarkerUpdate();

        /**
         set marker to start of animation
         */
        virtual void Rewind();

        /**
         get total duration in milliseconds of the animation
         */
        virtual double GetTotalLength() const;

        /**
         check if the animation is endless
         */
        virtual bool IsEndless() const;

        /**
         check if the animation did not yet reach its end
         */
        virtual bool IsPlaying() const;

        /**
         get the first position in milliseconds at which a value occurs
         */
        virtual double GetPositionFromValue(double value);

     protected:
        static void AddTweenType(JSONNode *node, const std::string &attribute_name, const TweenType &value);

        static void SetTweenTypeIfAttributeExists(const JSONNode *node, const std::string &attributeName, TweenType *value,
                                                  TweenType default_value);
    };

    /**
     shared pointer to an huestream::Animation object
     */
    SMART_POINTER_TYPES_FOR(Animation);

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_ANIMATION_H_
