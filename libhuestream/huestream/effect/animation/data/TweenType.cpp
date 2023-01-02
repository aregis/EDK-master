/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/TweenType.h>

#include <string>
#include <map>
#include <utility>

namespace huestream {

    std::string TweenTypeHelper::Linear = "Linear";
    std::string TweenTypeHelper::EaseInOutQuad = "EaseInOutQuad";
    std::string TweenTypeHelper::EaseInOutSine = "EaseInOutSine";

    std::map<TweenType, std::string> InitTweenToStringMap() {
        std::map<TweenType, std::string> map;
        map.insert(std::pair<TweenType, std::string>(TweenType::Linear, TweenTypeHelper::Linear));
        map.insert(std::pair<TweenType, std::string>(TweenType::EaseInOutQuad, TweenTypeHelper::EaseInOutQuad));
        map.insert(std::pair<TweenType, std::string>(TweenType::EaseInOutSine, TweenTypeHelper::EaseInOutSine));
        return map;
    }

    std::map<TweenType, std::string> TweenTypeHelper::tweenTypeToString_ = InitTweenToStringMap();

    std::map<std::string, TweenType> InitStringToTweenMap() {
        std::map<std::string, TweenType> map;
        map.insert(std::pair<std::string, TweenType>(TweenTypeHelper::Linear, TweenType::Linear));
        map.insert(std::pair<std::string, TweenType>(TweenTypeHelper::EaseInOutQuad, TweenType::EaseInOutQuad));
        map.insert(std::pair<std::string, TweenType>(TweenTypeHelper::EaseInOutSine, TweenType::EaseInOutSine));
        return map;
    }

    std::map<std::string, TweenType> TweenTypeHelper::stringToTweenType_ = InitStringToTweenMap();


    TweenType TweenTypeHelper::FromString(std::string s) {
        auto it = stringToTweenType_.find(s);
        if (it == stringToTweenType_.end())
            return stringToTweenType_.begin()->second;
        return it->second;
    }

    std::string TweenTypeHelper::ToString(TweenType t) {
        auto it = tweenTypeToString_.find(t);

        if (it == tweenTypeToString_.end())
            return tweenTypeToString_.begin()->second;

        return it->second;
    }
}  // namespace huestream
