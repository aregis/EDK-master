/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/effects/SequenceEffect.h>
#include <huestream/effect/animation/data/Vector.h>
#include <huestream/config/Config.h>
#include <huestream/effect/IMixer.h>

#include <algorithm>
#include <string>

namespace huestream {

    SequenceEffect::SequenceEffect(std::string name, unsigned int layer) :
            Effect(name, layer),
            _sequenceList(),
            _bri(0.5),
            _colors(),
            _stepCount(0),
            _mode(SequenceModeAll) {
        SetMode(SequenceModeAll);
    }

    bool SequenceEffect::AddArea(const Area &area) {
        auto found = false;
        if (_group == nullptr) {
            return false;
        }

        for (auto light : *_group->GetLights()) {
            if (area.isInArea(light->GetPosition())) {
                found = true;
            }
        }

        if (found) {
            _sequenceList.push_back(area);
        }
        return found;
    }

    void SequenceEffect::SetMode(SequenceMode mode) {
        _mode = mode;
        _sequenceList.clear();
        switch (_mode) {
            case SequenceModeAll:
                _sequenceList.push_back(Area::All);
                break;
            case SequenceModeRound:
                AddArea(Area::FrontLeft);
                if (!AddArea(Area::FrontCenter)) {
                    AddArea(Area::Center);
                }
                AddArea(Area::FrontRight);
                AddArea(Area::CenterRight);
                AddArea(Area::BackRight);
                AddArea(Area::BackCenter);
                AddArea(Area::BackLeft);
                AddArea(Area::CenterLeft);
                break;
            case SequenceModeFrontBack:
                AddArea(Area::FrontHalf);
                AddArea(Area::BackHalf);
                break;
            case SequenceModeLeftRight:
                AddArea(Area::Left);
                AddArea(Area::CenterLR);
                AddArea(Area::Right);
                break;
            default:
                break;
        }
    }

    SequenceMode SequenceEffect::GetMode() const {
        return _mode;
    }

    Color SequenceEffect::GetCurrentStepColor() {
        if (_colors.size() == 0)
            return Color();

        auto colorIndex = _stepCount % _colors.size();
        auto color = _colors[colorIndex];
        color.ApplyBrightness(_bri);
        return color;
    }


    Color SequenceEffect::GetColor(LightPtr light) {
        if (_colors.size() == 0)
            return Color();

        auto activeArea = _stepCount % _sequenceList.size();

        if (_sequenceList[activeArea].isInArea(light->GetPosition())) {
            return GetCurrentStepColor();
        }
        return Color();
    }

    void SequenceEffect::Step() {
        _stepCount++;
    }

    void SequenceEffect::AddColor(Color color) {
        _colors.push_back(color);
    }

    void SequenceEffect::SetBri(double bri) {
        _bri = bri;
    }

    double SequenceEffect::GetBri() const {
        return _bri;
    }

    void SequenceEffect::InceaseBri(double amount) {
        _bri = std::min(_bri + amount, 1.0);
    }

    void SequenceEffect::DecreaseBri(double amount) {
        _bri = std::max(_bri - amount, 0.0);
    }

    void SequenceEffect::Render() {
    }

    std::string SequenceEffect::GetTypeName() const {
        return type;
    }

    void SequenceEffect::UpdateGroup(GroupPtr group) {
        _group = group;
        SetMode(_mode);
    }

}  // namespace huestream
