/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/ProtocolSerializer.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace huestream {

    class TestProtocolSerializer : public testing::Test {
    public:
        std::shared_ptr<StreamOptions> _options;

        virtual void SetUp() {
            _options = std::make_shared<StreamOptions>();
            _options->colorSpace = COLORSPACE_RGB;

            _options->useClipV2 = false;

            auto group = std::make_shared<Group>();
            _options->group = group;

            group->AddLight("1", 0, 0);
            group->AddLight("2", 0, 0);

            _options->group->GetLights()->at(0)->SetColor(Color(1.0, 0, 0.02));
            _options->group->GetLights()->at(1)->SetColor(Color(-1.0, 0.2, 1.0));

        }

        virtual void TearDown() {
        }
    };

    TEST_F(TestProtocolSerializer, Serialize) {
        const uint8_t seqNr = 3;
        auto payload = ProtocolSerializer(_options).Serialize(seqNr);

        std::vector<uint8_t> expect;

        expect.push_back(static_cast<uint8_t>('H'));
        expect.push_back(static_cast<uint8_t>('u'));
        expect.push_back(static_cast<uint8_t>('e'));
        expect.push_back(static_cast<uint8_t>('S'));
        expect.push_back(static_cast<uint8_t>('t'));
        expect.push_back(static_cast<uint8_t>('r'));
        expect.push_back(static_cast<uint8_t>('e'));
        expect.push_back(static_cast<uint8_t>('a'));
        expect.push_back(static_cast<uint8_t>('m'));
        expect.push_back(static_cast<uint8_t>(_options->useClipV2 ? 2 : 1));
        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(seqNr));
        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(COLORSPACE_RGB));
        expect.push_back(static_cast<uint8_t>(0));

        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(1));
        expect.push_back(static_cast<uint8_t>(0xFF));
        expect.push_back(static_cast<uint8_t>(0xFF));
        expect.push_back(static_cast<uint8_t>(0x00));
        expect.push_back(static_cast<uint8_t>(0x00));
        expect.push_back(static_cast<uint8_t>(0x05));
        expect.push_back(static_cast<uint8_t>(0x1E));

        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(0));
        expect.push_back(static_cast<uint8_t>(2));
        expect.push_back(static_cast<uint8_t>(0x00));
        expect.push_back(static_cast<uint8_t>(0x00));
        expect.push_back(static_cast<uint8_t>(0x33));
        expect.push_back(static_cast<uint8_t>(0x33));
        expect.push_back(static_cast<uint8_t>(0xFF));
        expect.push_back(static_cast<uint8_t>(0xFF));

        ASSERT_THAT(payload, testing::ElementsAreArray(expect));
    }


}

