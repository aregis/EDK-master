/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_CUBOIDAREA_H_
#define HUESTREAM_COMMON_DATA_CUBOIDAREA_H_

#include "huestream/common/data/IArea.h"
#include "huestream/common/data/Location.h"
#include "huestream/common/serialize/SerializerHelper.h"

#include <vector>
#include <string>

namespace huestream {

    /**
     defintion of an area within (or outside when inverted) the cuboid spanned by a top front left and bottom back right location
     */
    class CuboidArea : public IArea {
    public:
        static constexpr const char* type = "huestream.CuboidArea";

        static HUESTREAM_EXPORT CuboidArea All;
        static HUESTREAM_EXPORT CuboidArea Front;
        static HUESTREAM_EXPORT CuboidArea Back;
        static HUESTREAM_EXPORT CuboidArea Left;
        static HUESTREAM_EXPORT CuboidArea CenterLR;
        static HUESTREAM_EXPORT CuboidArea Right;
        static HUESTREAM_EXPORT CuboidArea Top;
        static HUESTREAM_EXPORT CuboidArea CenterTB;
        static HUESTREAM_EXPORT CuboidArea Bottom;
        static HUESTREAM_EXPORT CuboidArea FrontLeft;
        static HUESTREAM_EXPORT CuboidArea FrontCenterLR;
        static HUESTREAM_EXPORT CuboidArea FrontRight;
        static HUESTREAM_EXPORT CuboidArea BackLeft;
        static HUESTREAM_EXPORT CuboidArea BackCenterLR;
        static HUESTREAM_EXPORT CuboidArea BackRight;
        static HUESTREAM_EXPORT CuboidArea FrontTop;
        static HUESTREAM_EXPORT CuboidArea FrontCenterTB;
        static HUESTREAM_EXPORT CuboidArea FrontBottom;
        static HUESTREAM_EXPORT CuboidArea FrontTopLeft;
        static HUESTREAM_EXPORT CuboidArea FrontTopCenter;
        static HUESTREAM_EXPORT CuboidArea FrontTopRight;
        static HUESTREAM_EXPORT CuboidArea FrontCenterLeft;
        static HUESTREAM_EXPORT CuboidArea FrontCenterCenter;
        static HUESTREAM_EXPORT CuboidArea FrontCenterRight;
        static HUESTREAM_EXPORT CuboidArea FrontBottomLeft;
        static HUESTREAM_EXPORT CuboidArea FrontBottomCenter;
        static HUESTREAM_EXPORT CuboidArea FrontBottomRight;

        static HUESTREAM_EXPORT std::vector<CuboidArea *> _knownAreas;

        static CuboidArea *GetKnownArea(std::string name);

        static std::vector<CuboidArea *> InitKnownAreas();

        CuboidArea();

        CuboidArea(const Location &topFrontLeft, const Location &bottomBackRight, const std::string name, bool inverted = false);

        AreaPtr Clone() const override;

        bool isInArea(const Location &location) const override;

    PROP_DEFINE(CuboidArea, Location, topFrontLeft, TopFrontLeft);
    PROP_DEFINE(CuboidArea, Location, bottomBackRight, BottomBackRight);
    PROP_DEFINE_BOOL(CuboidArea, bool, inverted, Inverted);
    PROP_DEFINE(CuboidArea, std::string, name, Name);

    public:
        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_CUBOIDAREA_H_
