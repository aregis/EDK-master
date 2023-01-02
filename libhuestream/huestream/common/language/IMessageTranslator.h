/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_LANGUAGE_IMESSAGETRANSLATOR_H_
#define HUESTREAM_COMMON_LANGUAGE_IMESSAGETRANSLATOR_H_

#include <memory>
#include <string>

namespace huestream {

    class IMessageTranslator {
    public:
        virtual ~IMessageTranslator() = default;

        virtual std::string Translate(std::string tag) const = 0;
    };

    typedef std::shared_ptr<IMessageTranslator> MessageTranslatorPtr;

}  // namespace huestream

#endif  // HUESTREAM_COMMON_LANGUAGE_IMESSAGETRANSLATOR_H_
