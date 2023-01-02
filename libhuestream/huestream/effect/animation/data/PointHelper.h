/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_POINTHELPER_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_POINTHELPER_H_

#include "huestream/effect/animation/data/Point.h"

namespace huestream {

    /**
    Helper class to create list of points
     */
    class PointHelper {
    public:
        static PointListPtr CreatePtr(PointPtr p1);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5, PointPtr p6);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5, PointPtr p6, PointPtr p7);

        static PointListPtr CreatePtr(PointPtr p1, PointPtr p2, PointPtr p3, PointPtr p4, PointPtr p5, PointPtr p6, PointPtr p7, PointPtr p8);
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_POINTHELPER_H_
