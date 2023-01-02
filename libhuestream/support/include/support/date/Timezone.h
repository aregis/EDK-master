/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

#include "support/util/StringUtil.h"

namespace support {

    class Timezone {
    public:
        /**
         Construct timezone by name
         @param name The name of the timezone
         */
        explicit Timezone(const std::string& name);
        
        /**
         Get UTC timezone
         @return The UTC timezone
         */
        static Timezone utc();
        
        /**
         Get local timezone, which represents the current
         configured timezone on the machine
         @return The local timezone
         */
        static Timezone local();
        
        /**
         Get name of the timezone
         @return The name of the timezone
         */
        const std::string& get_name() const;
    
    private:
        /** the name of the timezone */
        std::string _name;
    };

}  // namespace support

