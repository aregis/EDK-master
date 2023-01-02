/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/http/util/URLEncoder.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

namespace support {

    string URLEncoder::url_encode(string value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        
        for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
            string::value_type c = (*i);
            
            // Keep alphanumeric and other accepted characters intact
            if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
                continue;
            }
            
            // Any other characters are percent-encoded
            escaped << '%' << std::setw(2) << int((unsigned char) c);
        }
        
        return escaped.str();
    }
}  // namespace support
