/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <functional>

#include "support/util/Uuid.h"

namespace support {

    /**
     Convert value to string
     @param  value The value to be converted to a string
     @return The string value
     */
    template <typename Type>
    std::string to_string(const Type& value) {
        std::ostringstream string_stream;
        string_stream << value;
        
        return string_stream.str();
    }

    template<>
    std::string to_string<const boost::uuids::uuid>(const boost::uuids::uuid& id);
    
    /**
     Make string upper case (only characters a-z are supported)
     @param value The value to make upper case
     @return The string in upper case. If a certain character could not
             be converted, it will keep the old value
     */
    std::string to_upper_case(std::string value);

    /** see to_upper_case() */
    std::string to_lower_case(std::string value);
    
    /**
     Generate a random alpha-numeric string
     @param size The size of the random string
     @return The generated string
     */
    std::string random_alphanumeric_string(size_t size);
    
    /**
     Spit string by the symbol
     @param s The input string
     @param c The spitted symbol
     @result the vector with tokens
     */
    std::vector<std::string> split(const std::string& s, char c);
    
    /**
     Spit string by array of separators
     @param s The input string
     @param separators The Vector of separators
     @result the vector with tokens
     */
    std::vector<std::string> split(const std::string& s, const std::vector<std::string>& separators);
    
    std::string::size_type find_separator(const std::string& s, std::string::size_type start_position, const std::vector<std::string>& separators, std::string::size_type* separator_length);
    
    std::string replace_all(std::string subject, const std::string& search, const std::string& replace);
    
    std::string trim(std::string str, const std::string whitespace = " \t\r\n");

    std::string join(const std::vector<std::string>& elements, const std::string& delimiter);

    std::vector<std::string> map_vector(const std::vector<std::string>& elements, std::function<std::string(const std::string&)> mapping);

    /* truncate str to len bytes respecting utf8 character boundaries
     * automatically filters out invalid utf8 characters */
    std::string truncate_utf8(std::string str, size_t len);

    /* filter out invalid utf8 characters */
    std::string filter_utf8(const std::string& str);
}  // namespace support
