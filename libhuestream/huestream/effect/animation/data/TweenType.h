/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_TWEENTYPE_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_TWEENTYPE_H_

#include <string>
#include <map>

namespace huestream {

    /**
     type of tween trajectory
     @see easings.net
     */
    enum class TweenType {
        Linear,         ///< linear trajectory
        EaseInOutQuad,  ///< quadratic easing in and out trajectory
        EaseInOutSine   ///< easing in and out with a sine trajectory
    };

    class TweenTypeHelper {
        static std::map<TweenType, std::string> tweenTypeToString_;
        static std::map<std::string, TweenType> stringToTweenType_;
    public:
        static TweenType FromString(std::string s);

        static std::string ToString(TweenType t);

        static std::string Linear;
        static std::string EaseInOutQuad;
        static std::string EaseInOutSine;
    };


}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_TWEENTYPE_H_
