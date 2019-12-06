#pragma once
#include <unordered_map>
#include <string>

class ElegentFileReader;

class CSVFileIndex {
public:
    struct FileBlockInfo {
        uint64_t begin_offset_ = 0;
        uint64_t end_offset_ = 0;
        uint64_t record_count_ = 0;
    };

    void __vectorcall createIndex(ElegentFileReader&, const size_t);
    const FileBlockInfo& __vectorcall find(const std::string& key);

private:
    std::unordered_map<std::string, const FileBlockInfo> catalogue_;

    friend class CSVFileIndex_EmptyFile_Test;
    friend class CSVFileIndex_SmallFile_Test;
    friend class CSVFileIndex_LargeFile_Test;
};