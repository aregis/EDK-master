/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKENGINE_H_
#define LIBHUESTREAM_TEST_MOCKENGINE_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "huestream/effect/IMixer.h"

namespace huestream {

    class MockMixer : public IMixer {
    public:

        MOCK_METHOD1(AddEffect, void(EffectPtr effect));
        MOCK_METHOD1(AddEffectList, void(EffectListPtr effects));
        MOCK_METHOD0(GetGroup, GroupPtr());
        MOCK_METHOD0(Render, void());
        MOCK_METHOD1(SetGroup, void(const GroupPtr group));
        MOCK_METHOD1(GetEffectByName, EffectPtr(std::string name));
        MOCK_METHOD0(Lock, void());
        MOCK_METHOD0(Unlock, void());
    };

    class MockWrapperMixer : public IMixer {
    public:
        explicit MockWrapperMixer(const std::shared_ptr<MockMixer>& mock)
          : _mock(mock) {}

        void AddEffect(EffectPtr effect) {
            _mock->AddEffect(effect);
        }

        void AddEffectList(EffectListPtr effects) {
            _mock->AddEffectList(effects);
        }

        GroupPtr GetGroup() {
            return _mock->GetGroup();
        }

        void Render() {
            _mock->Render();
        }

        void SetGroup(const GroupPtr group) {
            _mock->SetGroup(group);
        }

        EffectPtr GetEffectByName(std::string name) {
            return _mock->GetEffectByName(name);
        }

        void Lock() {
            _mock->Lock();
        }

        void Unlock() {
            _mock->Unlock();
        }

    private:
        std::shared_ptr<MockMixer> _mock;
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKENGINE_H_
