/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "support/network/http/HttpResponse.h"

using std::string;
using std::map;

namespace support {

    const char* HttpResponse::EMPTY_STRING = "";

    HttpResponse::HttpResponse() :
    _status_code(0),
    _file_size(0),
    _local_port(0) {
    }

    HttpResponse::HttpResponse(unsigned int status_code, const char* body) :
    _status_code(status_code),
    _body(string(body)),
    _file_size(0),
    _local_port(0) {
    }

    HttpResponse::HttpResponse(unsigned int status_code, const char* body, size_t body_size) :
    _status_code(status_code),
    _body(string(body, body_size)),
    _file_size(0),
    _local_port(0) {
    }

    unsigned int HttpResponse::get_status_code() const {
        return _status_code;
    }

    void HttpResponse::set_status_code(unsigned int status_code) {
        _status_code = status_code;
    }

    std::string HttpResponse::get_body() const {
        return _body;
    }

    void HttpResponse::set_body(const char* body) {
        _body = body;
    }

    void HttpResponse::set_body(const string& body) {
        _body = body;
    }

    size_t HttpResponse::get_body_size() const {
        return _body.size();
    }

    const char* HttpResponse::get_header_field_value(const char* field_name)  const {
        if (field_name == nullptr) {
            return EMPTY_STRING;
        }
        std::string field_name_string(field_name);
        std::transform(field_name_string.begin(), field_name_string.end(), field_name_string.begin(), tolower);

        HttpFieldMap::const_iterator field_value = _header_fields.find(field_name_string);

        if (field_value ==  _header_fields.end()) {
            return EMPTY_STRING;
        }

        return field_value->second.c_str();
    }

    void HttpResponse::add_header_field(const char* name, const char* value) {
        std::string name_string(name);
        std::transform(name_string.begin(), name_string.end(), name_string.begin(), tolower);

        _header_fields[name_string] = value;
    }

    void HttpResponse::set_certificate_chain(const std::vector<std::string>& certificate_chain) {
        _certificate_chain = certificate_chain;
    }

    std::vector<std::string> HttpResponse::get_certificate_chain() const {
        return _certificate_chain;
    }

    void HttpResponse::set_md5_digest(const std::string& digest) {
        _md5_digest = digest;
    }

    std::string HttpResponse::get_md5_digest() const {
        return _md5_digest;
    }

    void HttpResponse::set_file_size(size_t fileSize) {
        _file_size = fileSize;
    }

    size_t HttpResponse::get_file_size() const {
        return _file_size;
    }

    void HttpResponse::set_local_port(long local_port) {
        _local_port = local_port;
    }

    long HttpResponse::get_local_port() const {
        return _local_port;
    }

    std::unique_ptr<IHttpResponse> HttpResponse::clone() const {
        auto response = std::unique_ptr<HttpResponse>(new HttpResponse(_status_code, _body.c_str(), _body.length()));

        for (auto&& p : _header_fields) {
            response->add_header_field(p.first.c_str(), p.second.c_str());
        }

        response->set_certificate_chain(_certificate_chain);
        return std::unique_ptr<IHttpResponse>(response.release());
    }


}  // namespace support
