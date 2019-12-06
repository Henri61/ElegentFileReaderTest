#pragma once

#include <fstream>
#include <vector>

#include "Ensure.h"

inline uint64_t operator"" _Kb(uint64_t size)
{
    return 1024 * size;
}

class ElegentFileReader {
    using Buffer = std::vector<char>;

public:
    using Record = std::vector<const char*>;

    explicit ElegentFileReader(uint32_t read_size, char delimiter, bool as_one = false, size_t max_fields = -1);

    explicit ElegentFileReader(const char delimiter) : ElegentFileReader(static_cast<uint32_t>(4_Kb), delimiter)
    { }

    explicit ElegentFileReader(const uint32_t read_size)
        : ElegentFileReader(read_size, '\t')
    { }

    explicit ElegentFileReader()
        : ElegentFileReader('\t')
    { }

    ~ElegentFileReader() = default;

    void __vectorcall open(const char* filename);
    void __vectorcall close();
    void __vectorcall seek(uint64_t filepos_requested);

    const Record& __vectorcall readRecord();
    void __vectorcall skipRecord();

    bool __vectorcall isOpen() const
    {
        return ifs_.is_open();
    }

    uint64_t ftell() const
    {
        return pos_of_buffer_in_file_ + pos_;
    }

    bool isEOF() const
    {
        return eof_;
        //return ftell() == file_size_;
    }

private:
    bool __vectorcall foundDelimiter() const;
    bool __vectorcall foundDelimiterOr_0() const;
    void __vectorcall restoreEolCharacter();
    bool __vectorcall skippedOverUnicodeSignature();

    size_t __vectorcall getPartialRecordLength_andAdjustPos();

    template<bool>
    void __vectorcall storeTerminatingNull();
    template<bool>
    bool __vectorcall isEndOfRecord();
    template <bool>
    bool __vectorcall moveToNextChar();

    template <bool>
    void __vectorcall replaceDelimiterWith_0()
    {} // specialized case in cpp

    template <bool>
    size_t __vectorcall handlePartialRecord();

    template <bool = false>
    void __vectorcall resizeBuffer(const size_t new_size)
    {
        buffer_.resize(new_size);
    } // specialized case in cpp
    template <bool>
    void __vectorcall clearRecordStartedInUnicodeSignature()
    {} // specialized case in cpp

    template <bool>
    void __vectorcall readBlock();

    template <bool>
    void __vectorcall checkForNewField()
    {} // specialized case in cpp

    template <bool>
    const Record& __vectorcall getNextRecord();

    uint64_t file_size_;
    std::ifstream ifs_;
    const uint32_t read_size_;

    Buffer buffer_;
    size_t pos_;
    bool eof_;
    uint64_t pos_of_buffer_in_file_;
    size_t pos_of_record_in_buffer_;
    char last_actual_character_;

    const char delimiter_;
    const bool multiple_delimiters_as_one_;
    const size_t max_fields_requested_;
    Record record_;

    friend class ElegentFileReader_SmallFile_Test;
    friend class FileReaderTest_TestPrivateInterface_Test;

};


