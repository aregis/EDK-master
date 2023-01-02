/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace support {
    
    class AES256 {
    public:
        /**
         Encrypt the data with the given key
         @param key  The key used to encrypt
         @param data The data to encrypt
         @return The encrypted data. If the data could not be encrypted it returns an empty string
         */
        static std::string encrypt(const std::string& key, const std::string& data);
        
        /**
         Decrypt the data with the given key
         @param key  The key used to decrypt
         @param data The data to decrypt
         @return The decrypted data. If the data could not be decrypted it returns an empty string
         */
        static std::string decrypt(const std::string& key, const std::string& data);
    };
    
}  // namespace support

