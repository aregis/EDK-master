/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_DATA_APIVERSION_H_
#define HUESTREAM_COMMON_DATA_APIVERSION_H_

#include <string>

namespace huestream {

    class ApiVersion {
    public:
        ApiVersion();
        explicit ApiVersion(std::string version);
        ApiVersion(int major, int minor, int build);

        void SetVersionString(const std::string& version);
        const std::string& GetVersionString() const;

        bool IsValid() const;
        int GetMajor() const;
        int GetMinor() const;
        int GetBuild() const;

        virtual ~ApiVersion();

    private:
        void ToString();
        void FromString();

        std::string _versionString;
        bool _valid;
        int _major;
        int _minor;
        int _build;
    };

    inline bool operator==(const ApiVersion& lhs, const ApiVersion& rhs) {
        return lhs.GetVersionString() == rhs.GetVersionString();
    }
    inline bool operator!=(const ApiVersion& lhs, const ApiVersion& rhs) { return !operator==(lhs, rhs); }

    inline bool operator< (const ApiVersion& lhs, const ApiVersion& rhs) {
        if (lhs.GetMajor() < rhs.GetMajor()) {
            return true;
        }
        if (lhs.GetMajor() == rhs.GetMajor() && lhs.GetMinor() < rhs.GetMinor()) {
            return true;
        }
        if (lhs.GetMajor() == rhs.GetMajor() && lhs.GetMinor() == rhs.GetMinor() && lhs.GetBuild() < rhs.GetBuild()) {
            return true;
        }
        return false;
    }
    inline bool operator> (const ApiVersion& lhs, const ApiVersion& rhs) { return  operator< (rhs, lhs); }
    inline bool operator<=(const ApiVersion& lhs, const ApiVersion& rhs) { return !operator> (lhs, rhs); }
    inline bool operator>=(const ApiVersion& lhs, const ApiVersion& rhs) { return !operator< (lhs, rhs); }

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_APIVERSION_H_
