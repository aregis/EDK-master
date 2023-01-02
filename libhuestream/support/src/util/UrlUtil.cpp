/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/util/UrlUtil.h"

#include <regex>

using support::UrlUtil;

std::string UrlUtil::get_host(const std::string &url) {
    std::string return_value;
    static std::regex url_regex("^(.*:\\/\\/)?([^:^/]*)");

    std::smatch result;
    if (regex_search(url, result, url_regex)) {
        return_value = result.str(2).c_str();
    }

    return return_value;
}