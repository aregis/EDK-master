/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/CuboidArea.h>

#include <string>
#include <vector>
#include <memory>

namespace huestream {

#define F1_3 (1.0/3.0)

    PROP_IMPL(CuboidArea, Location, topFrontLeft, TopFrontLeft);
    PROP_IMPL(CuboidArea, Location, bottomBackRight, BottomBackRight);
    PROP_IMPL_BOOL(CuboidArea, bool, inverted, Inverted);
    PROP_IMPL(CuboidArea, std::string, name, Name);

/*
 *
 *     +1 +-----------------------------+
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 * |   0  |--------------+--------------|
 * y      |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *     -1 +-----------------------------+
 *       -1              0             +1
 *                     x--->
 *
 *     +1 +---------+---------+---------+
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *    1/3 +---------+---------+---------+
 * ^      |         |         |         |
 * |   0  |         |   0,0   |         |
 * z      |         |         |         |
 *   -1/3 +---------+---------+---------+
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *     -1 +-----------------------------+
 *       -1       -1/3   0  +1/3       +1
 *                     x--->
 *
 * */

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

    CuboidArea CuboidArea::All = CuboidArea(Location(-1, 1, 1), Location(1, -1, -1), "All");

    CuboidArea CuboidArea::Front = CuboidArea(Location(-1, 1, 1), Location(1, 0, -1), "Front");
    CuboidArea CuboidArea::Back = CuboidArea(Location(-1, 0, 1), Location(1, -1, -1), "Back");

    CuboidArea CuboidArea::Left = CuboidArea(Location(-1, 1, 1), Location(-F1_3, -1, -1), "Left");
    CuboidArea CuboidArea::CenterLR = CuboidArea(Location(-F1_3, 1, 1), Location(F1_3, -1, -1), "CenterLR");
    CuboidArea CuboidArea::Right = CuboidArea(Location(F1_3, 1, 1), Location(1, -1, -1), "Right");
    CuboidArea CuboidArea::Top = CuboidArea(Location(-1, 1, 1), Location(1, -1, F1_3), "Top");
    CuboidArea CuboidArea::CenterTB = CuboidArea(Location(-1, 1, F1_3), Location(1, -1, -F1_3), "CenterTB");
    CuboidArea CuboidArea::Bottom = CuboidArea(Location(-1, 1, -F1_3), Location(1, -1, -1), "Bottom");

    CuboidArea CuboidArea::FrontLeft = CuboidArea(Location(-1, 1, 1), Location(-F1_3, 0, -1), "FrontLeft");
    CuboidArea CuboidArea::FrontCenterLR = CuboidArea(Location(-F1_3, 1, 1), Location(F1_3, 0, -1), "FrontCenterLR");
    CuboidArea CuboidArea::FrontRight = CuboidArea(Location(F1_3, 1, 1), Location(1, 0, -1), "FrontRight");
    CuboidArea CuboidArea::BackLeft = CuboidArea(Location(-1, 0, 1), Location(-F1_3, -1, -1), "BackLeft");
    CuboidArea CuboidArea::BackCenterLR = CuboidArea(Location(-F1_3, 0, 1), Location(F1_3, -1, -1), "BackCenterLR");
    CuboidArea CuboidArea::BackRight = CuboidArea(Location(F1_3, 0, 1), Location(1, -1, -1), "BackRight");
    CuboidArea CuboidArea::FrontTop = CuboidArea(Location(-1, 1, 1), Location(1, 0, F1_3), "FrontTop");
    CuboidArea CuboidArea::FrontCenterTB = CuboidArea(Location(-1, 1, F1_3), Location(1, 0, -F1_3), "FrontCenterTB");
    CuboidArea CuboidArea::FrontBottom = CuboidArea(Location(-1, 1, -F1_3), Location(1, 0, -1), "FrontBottom");

    CuboidArea CuboidArea::FrontTopLeft = CuboidArea(Location(-1, 1, 1), Location(-F1_3, 0, F1_3), "FrontTopLeft");
    CuboidArea CuboidArea::FrontTopCenter = CuboidArea(Location(-F1_3, 1, 1), Location(F1_3, 0, F1_3), "FrontTopCenter");
    CuboidArea CuboidArea::FrontTopRight = CuboidArea(Location(F1_3, 1, 1), Location(1, 0, F1_3), "FrontTopRight");
    CuboidArea CuboidArea::FrontCenterLeft = CuboidArea(Location(-1, 1, F1_3), Location(-F1_3, 0, -F1_3), "FrontCenterLeft");
    CuboidArea CuboidArea::FrontCenterCenter = CuboidArea(Location(-F1_3, 1, F1_3), Location(F1_3, 0, -F1_3), "FrontCenterCenter");
    CuboidArea CuboidArea::FrontCenterRight = CuboidArea(Location(F1_3, 1, F1_3), Location(1, 0, -F1_3), "FrontCenterRight");
    CuboidArea CuboidArea::FrontBottomLeft = CuboidArea(Location(-1, 1, -F1_3), Location(-F1_3, 0, -1), "FrontBottomLeft");
    CuboidArea CuboidArea::FrontBottomCenter = CuboidArea(Location(-F1_3, 1, -F1_3), Location(F1_3, 0, -1), "FrontBottomCenter");
    CuboidArea CuboidArea::FrontBottomRight = CuboidArea(Location(F1_3, 1, -F1_3), Location(1, 0, -1), "FrontBottomRight");

    std::vector<CuboidArea *> CuboidArea::InitKnownAreas() {
        std::vector<CuboidArea *> x;
        x.push_back(&CuboidArea::All);
        x.push_back(&CuboidArea::Front);
        x.push_back(&CuboidArea::Back);
        x.push_back(&CuboidArea::Left);
        x.push_back(&CuboidArea::CenterLR);
        x.push_back(&CuboidArea::Right);
        x.push_back(&CuboidArea::Top);
        x.push_back(&CuboidArea::CenterTB);
        x.push_back(&CuboidArea::Bottom);
        x.push_back(&CuboidArea::FrontLeft);
        x.push_back(&CuboidArea::FrontCenterLR);
        x.push_back(&CuboidArea::FrontRight);
        x.push_back(&CuboidArea::BackLeft);
        x.push_back(&CuboidArea::BackCenterLR);
        x.push_back(&CuboidArea::BackRight);
        x.push_back(&CuboidArea::FrontTop);
        x.push_back(&CuboidArea::FrontCenterTB);
        x.push_back(&CuboidArea::FrontBottom);
        x.push_back(&CuboidArea::FrontTopLeft);
        x.push_back(&CuboidArea::FrontTopCenter);
        x.push_back(&CuboidArea::FrontTopRight);
        x.push_back(&CuboidArea::FrontCenterLeft);
        x.push_back(&CuboidArea::FrontCenterCenter);
        x.push_back(&CuboidArea::FrontCenterRight);
        x.push_back(&CuboidArea::FrontBottomLeft);
        x.push_back(&CuboidArea::FrontBottomCenter);
        x.push_back(&CuboidArea::FrontBottomRight);
        return x;
    }

    std::vector<CuboidArea *> CuboidArea::_knownAreas = InitKnownAreas();

    CuboidArea *CuboidArea::GetKnownArea(std::string name) {
        for (auto area : _knownAreas) {
            if (area->_name == name) {
                return area;
            }
        }
        return nullptr;
    }

    CuboidArea::CuboidArea() : _topFrontLeft(0, 0, 0), _bottomBackRight(0, 0, 0), _inverted(false), _name("") {
    }

    CuboidArea::CuboidArea(const Location &topFrontLeft, const Location &bottomBackRight, const std::string name, bool inverted) {
        _topFrontLeft = topFrontLeft;
        _bottomBackRight = bottomBackRight;
        _name = name;
        _inverted = inverted;
    }

    AreaPtr CuboidArea::Clone() const {
        return std::make_shared<CuboidArea>(*this);
    }

    bool CuboidArea::isInArea(const Location &location) const {
        bool inArea = location.GetX() >= _topFrontLeft.GetX() && location.GetX() <= _bottomBackRight.GetX() &&
                      location.GetY() <= _topFrontLeft.GetY() && location.GetY() >= _bottomBackRight.GetY() &&
                      location.GetZ() <= _topFrontLeft.GetZ() && location.GetZ() >= _bottomBackRight.GetZ();

        if (_inverted) {
            inArea = !inArea;
        }
        return inArea;
    }

    std::string CuboidArea::GetTypeName() const {
        return type;
    }

    void CuboidArea::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, AttributeName, _name);
        SerializeAttribute(node, AttributeTopFrontLeft, std::make_shared<Location>(_topFrontLeft));
        SerializeAttribute(node, AttributeBottomBackRight, std::make_shared<Location>(_bottomBackRight));
        SerializeValue(node, AttributeInverted, _inverted);
    }

    void CuboidArea::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, AttributeName, &_name, "");
        _topFrontLeft = *DeserializeAttribute<Location>(node, AttributeTopFrontLeft, std::make_shared<Location>(_topFrontLeft));
        _bottomBackRight = *DeserializeAttribute<Location>(node, AttributeBottomBackRight, std::make_shared<Location>(_bottomBackRight));
        DeserializeValue(node, AttributeInverted, &_inverted, false);
    }

}  // namespace huestream
