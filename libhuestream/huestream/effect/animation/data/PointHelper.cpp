/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <huestream/effect/animation/data/PointHelper.h>

#include <memory>

namespace huestream {

PointListPtr PointHelper::CreatePtr(PointPtr p1) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    return list;
}

PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    list->push_back(p2);
    return list;
}

PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    return list;
}

PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    return list;
}

PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    list->push_back(p5);
    return list;
}

PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5, PointPtr p6) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    list->push_back(p5);
    list->push_back(p6);
    return list;
}

    PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5, PointPtr p6, PointPtr p7) {
    auto list = std::make_shared<PointList>();
    list->push_back(p1);
    list->push_back(p2);
    list->push_back(p3);
    list->push_back(p4);
    list->push_back(p5);
    list->push_back(p6);
    list->push_back(p7);
    return list;
}

PointListPtr PointHelper::CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5, PointPtr p6, PointPtr p7, PointPtr p8) {
    auto list = std::make_shared<PointList>();
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
