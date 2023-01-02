/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <curl/curl.h>
#include <mbedtls/ssl.h>

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <fstream>

#include "support/network/http/HttpResponse.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/HttpRequestParams.h"
#include "support/threading/ThreadPool.h"
#include "support/threading/QueueDispatcher.h"
#include "support/crypto/HashMD5.h"

namespace support {

    class CurlRequest {
    public:
        CurlRequest(const HttpRequestParams &data, HttpRequestCallback callback);
        ~CurlRequest();

        CURL *get_handle() const;

        void send_response(CURLcode curl_code);

        void wait_for_completion();

        void set_as_complete();

        std::string get_interface_name();

                bool expect_event_stream() const { return _expect_event_stream; }

    private:
        CURL *_curl;

        struct curl_slist *_header_list;
        struct curl_httppost *_form_post;

        /* response buffers */
        std::string _header_buffer;
        std::string _body_buffer;

        HttpRequestCallback _callback;

        std::string _interface_name;

        HttpRequestError _error;
        HttpResponse     _response;

        std::promise<void> _is_complete;
        std::future<void> _is_complete_future;

        std::promise<void> _callback_posted;
        std::future<void> _callback_posted_future;

        QueueDispatcher _dispatcher;

        /* mbedtls certificate setup */
        mbedtls_x509_crt _crt;
        mbedtls_x509_crl _crl;

        /* common name resolution */
        struct curl_slist *_resolve_list;

        /* retrieved server certificate chain */
        std::vector<std::string> _certificate_chain;

        bool _verify_ssl;
        bool _verify_common_name_manually;
        std::string _common_name;
        std::vector<std::string> _trusted_certs;

        HttpRequestProgressCallback _progress_callback;

                bool _expect_event_stream;
                bool _is_writing_header;
                bool _done_writing_header;

        /* curl will write error messages here */
        char _error_buffer[CURL_ERROR_SIZE];

        std::ofstream _file_to_write;
        size_t _file_to_write_size;
        HashMD5 _md5;

        void setup_options(const HttpRequestParams &data);

        void setup_tls(const HttpRequestParams &data);

        void setup_tls_common_name(const HttpRequestParams &data);

        void setup_proxy(const HttpRequestParams &data);

        void append_user_request_headers(const HttpRequestParams &data);

        void setup_request_headers();

        void setup_post_body(const HttpRequestParams &data);

        HttpRequestError::ErrorCode parse_error_code(CURLcode curl_code);

        void process_response_headers(HttpResponse& response);

        void cleanup();

                void send_stream_response();

        static CURLcode curl_sslctx_function(CURL* curl, void* sslctx, void* data);
        static int mbedtls_x509parse_verify(void* data, mbedtls_x509_crt* cert, int path_cnt, uint32_t* flags);
        static int curl_xferinfo_function(void *clientp,
                                          curl_off_t dltotal,   curl_off_t dlnow,
                                          curl_off_t ultotal,   curl_off_t ulnow);

                static size_t write_callback(char *ptr, size_t memb_size, size_t nmemb, void* userdata);
                static size_t write_file_callback(char *ptr, size_t memb_size, size_t nmemb, void* userdata);
                static size_t header_write_callback(char *ptr, size_t memb_size, size_t nmemb, void* userdata);
    };

}  // namespace support

