/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Area.h>

#include <string>
#include <vector>
#include <memory>

namespace huestream {

#define F1_3 (1.0/3.0)

    PROP_IMPL(Area, Location, topLeft, TopLeft);
    PROP_IMPL(Area, Location, bottomRight, BottomRight);
    PROP_IMPL_BOOL(Area, bool, inverted, Inverted);
    PROP_IMPL(Area, std::string, name, Name);

/*
 *
 *     +1 +-----------------------------+
 *        |              |              |
 *        |              |              |
 *        |              |              |
 *        |              |              |
 * ^      |              |              |
 * |   0  |--------------+--------------|
 * y      |              |              |
 *        |              |              |
 *        |              |              |
 *        |              |              |
 *        |              |              |
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
 * y      |         |         |         |
 *   -1/3 +---------+---------+---------+
 *        |         |         |         |
 *        |         |         |         |
 *        |         |         |         |
 *     -1 +-----------------------------+
 *       -1       -1/3   0  +1/3       +1
 *                     x--->
 *
 * */

    Area Area::FrontLeft = Area(-1, 1, -F1_3, F1_3, "FrontLeft");
    Area Area::FrontCenter = Area(-F1_3, 1, F1_3, F1_3, "FrontCenter");
    Area Area::FrontRight = Area(F1_3, 1, 1, F1_3, "FrontRight");

    Area Area::CenterLeft = Area(-1, F1_3, -F1_3, -F1_3, "CenterLeft");
    Area Area::Center = Area(-F1_3, F1_3, F1_3, -F1_3, "Center");
    Area Area::CenterRight = Area(F1_3, F1_3, 1, -F1_3, "CenterRight");

    Area Area::BackLeft = Area(-1, -F1_3, -F1_3, -1, "BackLeft");
    Area Area::BackCenter = Area(-F1_3, -F1_3, F1_3, -1, "BackCenter");
    Area Area::BackRight = Area(F1_3, -F1_3, 1, -1, "BackRight");

    Area Area::All = Area(-1, 1, 1, -1, "All");

    Area Area::FrontHalf = Area(-1, 1, 1, 0, "FrontHalf");
    Area Area::LeftHalf = Area(-1, 1, 0, -1, "LeftHalf");
    Area Area::BackHalf = Area(-1, 0, 1, -1, "BackHalf");
    Area Area::RightHalf = Area(0, 1, 1, -1, "RightHalf");

    Area Area::FrontLeftQuarter = Area(-1, 1, 0, 0, "FrontLeftQuarter");
    Area Area::FrontRightQuarter = Area(0, 1, 1, 0, "FrontRightQuarter");
    Area Area::BackRightQuarter = Area(0, 0, 1, -1, "BackRightQuarter");
    Area Area::BackLeftQuarter = Area(-1, 0, 0, -1, "BackLeftQuarter");

    Area Area::Left = Area(-1, 1, -F1_3, -1, "Left");
    Area Area::CenterLR = Area(-F1_3, 1, F1_3, -1, "CenterLR");
    Area Area::Right = Area(F1_3, 1, 1, -1, "Right");

    Area Area::Front = Area(-1, 1, 1, F1_3, "Front");
    Area Area::CenterFB = Area(-1, F1_3, 1, -F1_3, "CenterFB");
    Area Area::Back = Area(-1, -F1_3, 1, -1, "Back");

    Area Area::FrontHalfLeft = Area(-1, 1, -F1_3, 0, "FrontHalfLeft");
    Area Area::FrontHalfCenter = Area(-F1_3, 1, F1_3, 0, "FrontHalfCenter");
    Area Area::FrontHalfRight = Area(F1_3, 1, 1, 0, "FrontHalfRight");

    Area Area::LeftHalfFront = Area(-1, 1, 0, F1_3, "LeftHalfFront");
    Area Area::LeftHalfCenter = Area(-1, F1_3, 0, -F1_3, "LeftHalfCenter");
    Area Area::LeftHalfBack = Area(-1, -F1_3, 0, -1, "LeftHalfBack");

    Area Area::BackHalfLeft = Area(-1, 0, -F1_3, -1, "BackHalfLeft");
    Area Area::BackHalfCenter = Area(-F1_3, 0, F1_3, -1, "BackHalfCenter");
    Area Area::BackHalfRight = Area(F1_3, 0, 1, -1, "BackHalfRight");

    Area Area::RightHalfFront = Area(0, 1, 1, F1_3, "RightHalfFront");
    Area Area::RightHalfCenter = Area(0, F1_3, 1, -F1_3, "RightHalfCenter");
    Area Area::RightHalfBack = Area(0, -F1_3, 1, -1, "RightHalfBack");

    std::vector<Area *> Area::InitKnownAreas() {
        std::vector<Area *> x;
        x.push_back(&Area::All);
        x.push_back(&Area::FrontHalf);
        x.push_back(&Area::LeftHalf);
        x.push_back(&Area::BackHalf);
        x.push_back(&Area::RightHalf);
        x.push_back(&Area::Front);
        x.push_back(&Area::CenterFB);
        x.push_back(&Area::Back);
        x.push_back(&Area::Left);
        x.push_back(&Area::CenterLR);
        x.push_back(&Area::Right);
        x.push_back(&Area::FrontLeft);
        x.push_back(&Area::FrontCenter);
        x.push_back(&Area::FrontRight);
        x.push_back(&Area::CenterLeft);
        x.push_back(&Area::Center);
        x.push_back(&Area::CenterRight);
        x.push_back(&Area::BackLeft);
        x.push_back(&Area::BackCenter);
        x.push_back(&Area::BackRight);
        x.push_back(&Area::FrontLeftQuarter);
        x.push_back(&Area::FrontRightQuarter);
        x.push_back(&Area::BackRightQuarter);
        x.push_back(&Area::BackLeftQuarter);
        x.push_back(&Area::FrontHalfLeft);
        x.push_back(&Area::FrontHalfCenter);
        x.push_back(&Area::FrontHalfRight);
        x.push_back(&Area::LeftHalfFront);
        x.push_back(&Area::LeftHalfCenter);
        x.push_back(&Area::LeftHalfBack);
        x.push_back(&Area::BackHalfLeft);
        x.push_back(&Area::BackHalfCenter);
        x.push_back(&Area::BackHalfRight);
        x.push_back(&Area::RightHalfFront);
        x.push_back(&Area::RightHalfCenter);
        x.push_back(&Area::RightHalfBack);
        return x;
    }

    std::vector<Area *> Area::_knownAreas = InitKnownAreas();

    Area *Area::GetKnownArea(std::string name) {
        for (auto area : _knownAreas) {
            if (area->_name == name) {
                return area;
            }
        }
        return nullptr;
    }

    Area::Area() : _topLeft(0, 0), _bottomRight(0, 0), _inverted(false), _name("") {
    }

    Area::Area(double topLeftX, double topLeftY, double bottomRightX, double bottomRightY, const std::string name,
               bool inverted) :
            _topLeft(Location(topLeftX, topLeftY)), _bottomRight(Location(bottomRightX, bottomRightY)),
            _inverted(inverted), _name(name) {
    }

    Area::Area(const Location &topLeft, const Location &bottomRight, const std::string name, bool inverted) {
        _topLeft = topLeft;
        _bottomRight = bottomRight;
        _name = name;
        _inverted = inverted;
    }

    AreaPtr Area::Clone() const {
        return std::make_shared<Area>(*this);
    }

    bool Area::isInArea(const Location &location) const {
        bool inArea = false;

        inArea = location.GetX() >= _topLeft.GetX() && location.GetX() <= _bottomRight.GetX() &&
                 location.GetY() <= _topLeft.GetY() && location.GetY() >= _bottomRight.GetY();

        if (_inverted) {
            inArea = !inArea;
        }
        return inArea;
    }

    std::string Area::GetTypeName() const {
        return type;
    }

    void Area::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, AttributeName, _name);
        SerializeAttribute(node, AttributeTopLeft, std::make_shared<Location>(_topLeft));
        SerializeAttribute(node, AttributeBottomRight, std::make_shared<Location>(_bottomRight));
        SerializeValue(node, AttributeInverted, _inverted);
    }

    void Area::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, AttributeName, &_name, "");
        _topLeft = *DeserializeAttribute<Location>(node, AttributeTopLeft, std::make_shared<Location>(_topLeft));
        _bottomRight = *DeserializeAttribute<Location>(node, AttributeBottomRight, std::make_shared<Location>(_bottomRight));
        DeserializeValue(node, AttributeInverted, &_inverted, false);
    }

}  // namespace huestream
