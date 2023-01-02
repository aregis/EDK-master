/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <huestream/effect/animation/animations/base/AnimationHelper.h>

namespace huestream {

AnimationListPtr AnimationHelper::CreatePtr(AnimationPtr p1) {
    auto list = std::make_shared<AnimationList>();
    list->push_back(p1);
    return list;
}

AnimationListPtr AnimationHelper::CreatePtr(AnimationPtr p1, AnimationPtr p2) {
    auto list = std::make_shared<AnimationList>();
    list->push_back(p1);
    list->push_back(p2);
    return list;
}

AnimationListPtr AnimationHelper::CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4) {
    auto list = std::make_shared<AnimationList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    return list;
}

AnimationListPtr AnimationHelper::CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4, AnimationPtr p5) {
    auto list = std::make_shared<AnimationList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    list->push_back(p5);
    return list;
}

AnimationListPtr AnimationHelper::CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4, AnimationPtr p5, AnimationPtr p6, AnimationPtr p7) {
    auto list = std::make_shared<AnimationList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    list->push_back(p5);
    list->push_back(p6);
    list->push_back(p7);
    return list;
}

AnimationListPtr AnimationHelper::CreatePtr(AnimationPtr p1, AnimationPtr p2, AnimationPtr p3, AnimationPtr p4, AnimationPtr p5, AnimationPtr p6, AnimationPtr p7, AnimationPtr p8) {
    auto list = std::make_shared<AnimationList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    list->push_back(p5);
    list->push_back(p6);
    list->push_back(p7);
    list->push_back(p8);
    return list;
}

}  // namespace huestream
