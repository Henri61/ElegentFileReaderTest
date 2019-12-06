#include "pch.h"

#include "CSVFileIndex.h"
#include "ElegentFileReader.h"
#include "Ensure.h"

void __vectorcall CSVFileIndex::createIndex(ElegentFileReader& reader, const size_t key_field_index)
{
    reader.seek(0);
    if (reader.isEOF())
        return;

    // skip the header row
    reader.skipRecord();
    if (reader.isEOF())
        return;

    FileBlockInfo file_block;
    file_block.begin_offset_ = reader.ftell();
    auto file_record = reader.readRecord();
    file_block.end_offset_ = reader.ftell();
    file_block.record_count_ = 1;
    std::string current_key = file_record[key_field_index];

    while (!reader.isEOF()) {
        file_record = reader.readRecord();
        const char* key = file_record[key_field_index];
        if (strcmp(key, current_key.c_str()) == 0) {
            file_block.end_offset_ = reader.ftell();
            ++file_block.record_count_;
            continue;
        }

        // store the catalogue entry
        ENSURE(catalogue_.emplace(current_key, file_block).second);

        // reset for next entry
        file_block.begin_offset_ = file_block.end_offset_; // end of previous block is the start of the next block
        file_block.end_offset_ = reader.ftell();
        file_block.record_count_ = 1;
        current_key = key;
    }

    ENSURE(catalogue_.emplace(current_key, file_block).second);
}

namespace {
    const CSVFileIndex::FileBlockInfo not_found;
}

const CSVFileIndex::FileBlockInfo& __vectorcall CSVFileIndex::find(const std::string& key)
{
    const auto iter = catalogue_.find(key);
    return iter == catalogue_.end() ? not_found : iter->second;
}
