/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/ProtocolSerializer.h>

#include <memory>
#include <vector>

namespace huestream {

#define RESERVED 0x00
#define VERSION_MINOR 0x00
#define ADDRTYPE_LIGHTID 0x00
#define ADDRTYPE_GROUPID 0x01

    ProtocolSerializer::ProtocolSerializer(std::shared_ptr<StreamOptions> options)
            : _options(options) {
    }

    ProtocolSerializer::~ProtocolSerializer() {
    }


    std::vector<uint8_t> ProtocolSerializer::Serialize(uint8_t seqNr) {
        std::vector<uint8_t> payload;

        payload.push_back(static_cast<uint8_t>('H'));
        payload.push_back(static_cast<uint8_t>('u'));
        payload.push_back(static_cast<uint8_t>('e'));
        payload.push_back(static_cast<uint8_t>('S'));
        payload.push_back(static_cast<uint8_t>('t'));
        payload.push_back(static_cast<uint8_t>('r'));
        payload.push_back(static_cast<uint8_t>('e'));
        payload.push_back(static_cast<uint8_t>('a'));
        payload.push_back(static_cast<uint8_t>('m'));
        payload.push_back(static_cast<uint8_t>(_options->useClipV2 ? 0x02 : 0x01));
        payload.push_back(static_cast<uint8_t>(VERSION_MINOR));
        payload.push_back(static_cast<uint8_t>(seqNr));
        payload.push_back(static_cast<uint8_t>(RESERVED));
        payload.push_back(static_cast<uint8_t>(RESERVED));
        payload.push_back(static_cast<uint8_t>(_options->colorSpace));
        payload.push_back(static_cast<uint8_t>(RESERVED));

        if (_options->useClipV2) {
            const std::string& id = _options->group->GetId();
            for (int i = 0; i < id.size(); ++i) {
                payload.push_back(static_cast<uint8_t>(id.at(i)));
            }
        }

        for (auto l : *_options->group->GetLights()) {
            auto id = static_cast<uint16_t>(std::stoul(l->GetId()));

            auto color = l->GetColor();
            color.Clamp();
            auto r = static_cast<uint16_t>(color.GetR() * 65535);
            auto g = static_cast<uint16_t>(color.GetG() * 65535);
            auto b = static_cast<uint16_t>(color.GetB() * 65535);

            if (!_options->useClipV2) {
                if (id >= 100) {
                    payload.push_back(static_cast<uint8_t>(ADDRTYPE_GROUPID));
                    id = id - 100;
                }
                else {
                    payload.push_back(static_cast<uint8_t>(ADDRTYPE_LIGHTID));
                }
            }

            if (!_options->useClipV2) {
                payload.push_back(static_cast<uint8_t>((id & 0xff00) >> 8));
            }
            payload.push_back(static_cast<uint8_t>(id & 0x00ff));
            payload.push_back(static_cast<uint8_t>((r & 0xff00) >> 8));
            payload.push_back(static_cast<uint8_t>(r & 0x00ff));
            payload.push_back(static_cast<uint8_t>((g & 0xff00) >> 8));
            payload.push_back(static_cast<uint8_t>(g & 0x00ff));
            payload.push_back(static_cast<uint8_t>((b & 0xff00) >> 8));
            payload.push_back(static_cast<uint8_t>(b & 0x00ff));
        }

        return payload;
    }
}  // namespace huestream
