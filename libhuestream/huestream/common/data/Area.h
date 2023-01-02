/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_AREA_H_
#define HUESTREAM_COMMON_DATA_AREA_H_

#include "huestream/common/data/IArea.h"
#include "huestream/common/data/Location.h"
#include "huestream/common/serialize/SerializerHelper.h"

#include <vector>
#include <string>

namespace huestream {

    /**
     defintion of an area within (or outside when inverted) the rectangle spanned by a top left and bottom right location
     */
    class Area : public IArea {
    public:
        static constexpr const char* type = "huestream.Area";

        static HUESTREAM_EXPORT Area All;
        static HUESTREAM_EXPORT Area FrontHalf;
        static HUESTREAM_EXPORT Area BackHalf;
        static HUESTREAM_EXPORT Area LeftHalf;
        static HUESTREAM_EXPORT Area RightHalf;
        static HUESTREAM_EXPORT Area Front;
        static HUESTREAM_EXPORT Area CenterFB;
        static HUESTREAM_EXPORT Area Back;
        static HUESTREAM_EXPORT Area Left;
        static HUESTREAM_EXPORT Area CenterLR;
        static HUESTREAM_EXPORT Area Right;
        static HUESTREAM_EXPORT Area FrontLeft;
        static HUESTREAM_EXPORT Area FrontCenter;
        static HUESTREAM_EXPORT Area FrontRight;
        static HUESTREAM_EXPORT Area CenterLeft;
        static HUESTREAM_EXPORT Area Center;
        static HUESTREAM_EXPORT Area CenterRight;
        static HUESTREAM_EXPORT Area BackLeft;
        static HUESTREAM_EXPORT Area BackCenter;
        static HUESTREAM_EXPORT Area BackRight;
        static HUESTREAM_EXPORT Area FrontLeftQuarter;
        static HUESTREAM_EXPORT Area FrontRightQuarter;
        static HUESTREAM_EXPORT Area BackRightQuarter;
        static HUESTREAM_EXPORT Area BackLeftQuarter;
        static HUESTREAM_EXPORT Area FrontHalfLeft;
        static HUESTREAM_EXPORT Area FrontHalfCenter;
        static HUESTREAM_EXPORT Area FrontHalfRight;
        static HUESTREAM_EXPORT Area LeftHalfFront;
        static HUESTREAM_EXPORT Area LeftHalfCenter;
        static HUESTREAM_EXPORT Area LeftHalfBack;
        static HUESTREAM_EXPORT Area BackHalfLeft;
        static HUESTREAM_EXPORT Area BackHalfCenter;
        static HUESTREAM_EXPORT Area BackHalfRight;
        static HUESTREAM_EXPORT Area RightHalfFront;
        static HUESTREAM_EXPORT Area RightHalfCenter;
        static HUESTREAM_EXPORT Area RightHalfBack;

        static HUESTREAM_EXPORT std::vector<Area *> _knownAreas;

        static Area *GetKnownArea(std::string name);

        static std::vector<Area *> InitKnownAreas();

        Area();

        Area(double topLeftX, double topLeftY, double bottomRightX, double bottomRightY, const std::string name,
             bool inverted = false);

        Area(const Location &topLeft, const Location &bottomRight, const std::string name, bool inverted = false);

        AreaPtr Clone() const override;

        bool isInArea(const Location &location) const override;

    PROP_DEFINE(Area, Location, topLeft, TopLeft);
    PROP_DEFINE(Area, Location, bottomRight, BottomRight);
    PROP_DEFINE_BOOL(Area, bool, inverted, Inverted);
    PROP_DEFINE(Area, std::string, name, Name);

    public:
        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;
    };

    typedef Area RectangleArea;

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_AREA_H_
