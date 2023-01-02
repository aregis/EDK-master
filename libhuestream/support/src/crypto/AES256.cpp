/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <mbedtls/aes.h>

#include <cassert>
#include <cstring>
#include <string>

#include "support/logging/Log.h"
#include "support/crypto/AES256.h"

#define AES256_KEY_SIZE_BYTES 32
#define AES256_KEY_SIZE_BITS 256
#define AES256_ECB_BLOCK_SIZE 16

namespace support {

    std::string AES256::encrypt(const std::string& key, const std::string& data) {
        if (key.size() != AES256_KEY_SIZE_BYTES) {
            HUE_LOG << HUE_SUPPORT << HUE_ERROR << "AES256::encrypt - invalid key size" << HUE_ENDL;
            return std::string("");
        }

        if (data.size() == 0) {
            // special case for empty string to avoid passing zero length arrays to memcpy
            return std::string("\x00", 1);
        }

        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_enc(&ctx, reinterpret_cast<const unsigned char*>(key.c_str()), AES256_KEY_SIZE_BITS);

        size_t padding = 0;

        if ((data.size() % AES256_ECB_BLOCK_SIZE) > 0) {
            padding = AES256_ECB_BLOCK_SIZE - (data.size() % AES256_ECB_BLOCK_SIZE);
        }

        size_t padded_size = data.size() + padding;

        unsigned char* input = new unsigned char[padded_size];
        unsigned char* output = new unsigned char[padded_size];

        // we have to encrypt the data in blocks of 16 bytes, therefore if
        // the data size is not a multiple of 16, we pad the end with zeroes

        memcpy(input, data.c_str(), data.size());
        if (padding > 0) {
            memset(input + data.size(), 0, padding);
        }

        for (size_t offset = 0; offset < padded_size; offset += AES256_ECB_BLOCK_SIZE) {
            mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, input + offset, output + offset);
        }

        std::string cyphertext = std::string(reinterpret_cast<const char*>(output), padded_size);

        delete [] output;
        delete [] input;
        mbedtls_aes_free(&ctx);

        std::string ciphertext_with_prepended_padding;
        
        // Append the padding size to the front of the string to get exactly the same output as the old AES library
        ciphertext_with_prepended_padding.insert(0, 1, static_cast<char>(padding));
        ciphertext_with_prepended_padding.insert(1, cyphertext);
        
        return ciphertext_with_prepended_padding;
    }

    std::string AES256::decrypt(const std::string& key, const std::string& data) {
        if (key.size() != AES256_KEY_SIZE_BYTES) {
            HUE_LOG << HUE_SUPPORT << HUE_ERROR << "AES256::decrypt - invalid key size" << HUE_ENDL;
            return std::string("");
        }

        if ((data.size() % AES256_ECB_BLOCK_SIZE) != 1) {
            HUE_LOG << HUE_SUPPORT << HUE_ERROR << "AES256::decrypt - cyphertext legth not a multiple of the block size" << HUE_ENDL;
            return std::string("");
        }

        // the cyphertext is prepended by one byte that indicates the padding

        unsigned char padding = static_cast<unsigned char>(data[0]);

        if (padding > 15 || (data.size() == 1 && padding != 0)) {
            HUE_LOG << HUE_SUPPORT << HUE_ERROR << "AES256::decrypt - invalid padding" << HUE_ENDL;
            return std::string("");
        }

        std::string cyphertext = data.substr(1);

        const unsigned char* input = reinterpret_cast<const unsigned char*>(cyphertext.c_str());
        unsigned char* output = new unsigned char[cyphertext.size()];

        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_dec(&ctx, reinterpret_cast<const unsigned char*>(key.c_str()), AES256_KEY_SIZE_BITS);

        for (size_t offset = 0; offset < cyphertext.size(); offset += AES256_ECB_BLOCK_SIZE) {
            mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, input + offset, output + offset);
        }

        std::string plaintext = std::string(reinterpret_cast<char *>(output), cyphertext.size() - padding);

        delete [] output;
        mbedtls_aes_free(&ctx);

        return plaintext;
    }

}  // namespace support
