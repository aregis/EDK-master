/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/FramesAnimation.h>
#include <huestream/common/util/HueMath.h>

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

namespace huestream {

    std::string FramesAnimation::AttributeFps = "Fps";
    double FramesAnimation::GetFps() const { return _fpms * 1000.0; }
    void FramesAnimation::SetFps(const double &fps) { _fpms = fps / 1000.0; _frameLength = 1000.0 / fps; }

    PROP_IMPL(FramesAnimation, std::shared_ptr<std::vector<uint16_t>>, frames, Frames);
    PROP_IMPL(FramesAnimation, bool, compressionEnabled, CompressionEnabled);

    FramesAnimation::FramesAnimation() : FramesAnimation(24) {}

    FramesAnimation::FramesAnimation(double fps)
            : RepeatableAnimation(0), _frames(std::make_shared<std::vector<uint16_t>>()), _compressionEnabled(true) {
        SetFps(fps);
    }

    FramesAnimation::FramesAnimation(double fps, unsigned int size) : FramesAnimation(fps) {
        _frames->reserve(size);
    }
            
    AnimationPtr FramesAnimation::Clone() {
        return std::make_shared<FramesAnimation>(*this);
    }

    void FramesAnimation::UpdateValue(double *value, double positionMs) {
        if (_frames->size() == 0)
            return;

        auto fracIndex = positionMs * _fpms;
        auto roundIndex = std::floor(fracIndex);
        auto index = static_cast<std::vector<uint16_t>::size_type>(roundIndex);

        if (index >= _frames->size() - 1) {
            *value = static_cast<double>(_frames->back()) / UINT16_MAX;
        } else {
            auto position = (fracIndex - roundIndex) * _frameLength;
            auto start = static_cast<double>((*_frames)[index]) / UINT16_MAX;
            auto end = static_cast<double>((*_frames)[index + 1]) / UINT16_MAX;
            *value = HueMath::linearTween(position, start, end, _frameLength);
        }
    }

    double FramesAnimation::GetLengthMs() const {
        return _frames->size() * _frameLength;
    }

    void FramesAnimation::Append(const double frame) {
        _frames->push_back(static_cast<uint16_t>(frame * UINT16_MAX));
    }

    void FramesAnimation::Serialize(JSONNode *node) const {
        RepeatableAnimation::Serialize(node);

        SerializeValue(node, AttributeFps, _fpms * 1000.0);

        if (_compressionEnabled) {
            SerializeFramesCompressed(node);
        } else {
            SerializeFramesRaw(node);
        }
    }

    void FramesAnimation::Deserialize(JSONNode *node) {
        RepeatableAnimation::Deserialize(node);

        if (SerializerHelper::IsAttributeSet(node, AttributeFps)) {
            auto jsonFps = (*node)[AttributeFps];
            SetFps(jsonFps.as_float());
        }

        if (SerializerHelper::IsAttributeSet(node, AttributeFrames)) {
            auto jsonFrames = (*node)[AttributeFrames];

            _frames->clear();
            if (jsonFrames.type() == JSON_ARRAY) {
                _frames->reserve(jsonFrames.size());
                for (auto &jsonFrame : jsonFrames) {
                    Append(jsonFrame.as_float());
                }
            } else if (jsonFrames.type() == JSON_STRING) {
                _frames->reserve(jsonFrames.size() / 2);
                AddFramesFrom12BitBase64String(jsonFrames.as_string());
            }
        }
    }

    std::string FramesAnimation::GetTypeName() const {
        return type;
    }

    void FramesAnimation::SerializeFramesCompressed(JSONNode *node) const {
        std::ostringstream oss;
        for (auto &f : *_frames) {
            oss << Pack12bitBase64(static_cast<double>(f) / UINT16_MAX);
        }
        JSONNode framesNode(AttributeFrames, oss.str());
        node->push_back(framesNode);
    }

    void FramesAnimation::SerializeFramesRaw(JSONNode *node) const {
        JSONNode framesNode(JSON_ARRAY);
        for (auto &f : *_frames) {
            framesNode.push_back(JSONNode("", static_cast<double>(f) / UINT16_MAX));
        }
        framesNode.set_name(AttributeFrames);
        node->push_back(framesNode);
    }

    void FramesAnimation::AddFramesFrom12BitBase64String(const std::string& frameString) {
        int value = 0;
        bool haveMSB = false;

        for (const char& c : frameString) {
            int temp = 0;
            if (c >= 'A' && c <= 'Z')
                temp = c - 'A';
            else if (c >= 'a' && c <= 'z')
                temp = c - 'a' + 26;
            else if (c >= '0' && c <= '9')
                temp = c - '0' + 52;
            else if (c == '+')
                temp = 62;
            else if (c == '/')
                temp = 63;

            if (haveMSB) {
                value |= temp;
                Append(static_cast<double>(value) / 4095.0);
                value = 0;
                haveMSB = false;
            }
            else {
                value = (temp << 6);
                haveMSB = true;
            }
        }
    }

    const std::string FramesAnimation::Pack12bitBase64(double number) const {
        //input number between 0 and 1
        std::string numerals = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        auto rounded = static_cast<int>(std::roundf(number * 4095.0));
        std::ostringstream oss;
        oss << numerals.at(rounded >> 6) << numerals.at(rounded & 0x3f);
        return oss.str();
    }

}  // namespace huestream
