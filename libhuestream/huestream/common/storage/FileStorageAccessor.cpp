/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/storage/FileStorageAccessor.h>

#include <fstream>
#include <sstream>
#include <string>
#include <codecvt>

namespace huestream {

    FileStorageAccessor::FileStorageAccessor(const std::string &fileName) : _fileName(fileName) {
    }

    void FileStorageAccessor::Load(LoadCallbackHandler cb) {
        std::fstream file;
#ifdef WIN32
        // We need to use the wstring version of this function otherwise if the file name contain non ascii char, it will fail on Windows
        file.open(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(_fileName), std::fstream::in);
#else
        file.open(_fileName.c_str(), std::fstream::in);
#endif
        if (file.is_open()) {
            std::ostringstream contents;
            contents << file.rdbuf();
            file.close();

            JSONNode node = libjson::parse(contents.str());
            auto s = Serializable::DeserializeFromJson(&node);
            cb(OPERATION_SUCCESS, s);
        } else {
            cb(OPERATION_FAILED, nullptr);
        }
    }

    void FileStorageAccessor::Save(SerializablePtr serializable, SaveCallbackHandler cb) {
        JSONNode node;
        serializable->Serialize(&node);
        std::ofstream file;
#ifdef WIN32
        // We need to use the wstring version of this function otherwise if the file name contain non ascii char, it will fail on Windows
        file.open(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(_fileName), std::fstream::out | std::fstream::trunc);
#else
        file.open(_fileName.c_str(), std::fstream::out | std::fstream::trunc);
#endif
        if (file.is_open()) {
            std::string jc = node.write_formatted();
            std::ostringstream contents(jc);
            file << contents.str();
            file.close();
            cb(OPERATION_SUCCESS);
        } else {
            cb(OPERATION_FAILED);
        }
    }
}  // namespace huestream
