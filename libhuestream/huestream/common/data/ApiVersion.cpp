/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/ApiVersion.h>

#include <string>
#include <regex>
#include <sstream>

namespace huestream {

ApiVersion::ApiVersion()
    : _versionString(""), _valid(false), _major(0), _minor(0), _build(0) {
}

ApiVersion::ApiVersion(int major, int minor, int build)
    : _versionString(""), _valid(true), _major(major), _minor(minor), _build(build) {
    ToString();
}

ApiVersion::ApiVersion(std::string version)
    : _versionString(version), _valid(false), _major(0), _minor(0), _build(0) {
    FromString();
}

ApiVersion::~ApiVersion() {
}

void ApiVersion::SetVersionString(const std::string& version) {
    _versionString = version;
    FromString();
}

const std::string& ApiVersion::GetVersionString() const {
    return _versionString;
}

bool ApiVersion::IsValid() const { return _valid; }
int ApiVersion::GetMajor() const { return _major; }
int ApiVersion::GetMinor() const { return _minor; }
int ApiVersion::GetBuild() const { return _build; }

void ApiVersion::FromString() {
    std::regex re("(\\d+)\\.(\\d+)\\.(\\d+)");
    std::smatch match;
    if (!std::regex_search(_versionString, match, re) || match.size() != 4) {
        _valid = false;
        return;
    }
    _valid = true;
    _major = std::stoi(match.str(1));
    _minor = std::stoi(match.str(2));
    _build = std::stoi(match.str(3));
}

void ApiVersion::ToString() {
    std::ostringstream oss;
    oss << _major << "." << _minor << "." << _build;
    _versionString = oss.str();
}

}  // namespace huestream
