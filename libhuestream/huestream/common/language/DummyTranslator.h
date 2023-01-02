/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_LANGUAGE_DUMMYTRANSLATOR_H_
#define HUESTREAM_COMMON_LANGUAGE_DUMMYTRANSLATOR_H_

#include "huestream/common/language/IMessageTranslator.h"

#include <string>
#include <map>

namespace huestream {

    class DummyTranslator : public IMessageTranslator {
    public:
        explicit DummyTranslator(std::string language);

        virtual ~DummyTranslator() = default;

        std::string Translate(std::string tag) const override;

    protected:
        static std::map<std::string, std::map<std::string, std::string>> translations;
        static std::map<std::string, std::string> english;
        static std::map<std::string, std::string> dutch;
        static std::map<std::string, std::string> german;
    private:
        std::string _language;
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_LANGUAGE_DUMMYTRANSLATOR_H_
