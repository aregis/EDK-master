/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKTMESSAGETRANSLATOR_H_
#define LIBHUESTREAM_TEST_MOCKTMESSAGETRANSLATOR_H_

#include <huestream/common/language/IMessageTranslator.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace huestream {

    class MockMessageTranslator : public IMessageTranslator {
    public:
        MOCK_CONST_METHOD1(Translate, std::string(std::string tag));
    };

    class MockWrapperMessageTranslator : public IMessageTranslator {
    public:
        explicit MockWrapperMessageTranslator(const std::shared_ptr<MockMessageTranslator> mock)
          : _mock(mock) {}

        std::string Translate(std::string tag) const {
            return _mock->Translate(tag);
        }

    private:
        std::shared_ptr<MockMessageTranslator> _mock;
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKTMESSAGETRANSLATOR_H_
