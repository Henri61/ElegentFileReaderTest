#include "pch.h"

#include "ElegentFileReader.h"

#include <exception>
#include <string>
#include <Windows.h>

#include "Ensure.h"
#include <filesystem>

namespace {
const bool build_record = true;
const char CR = '\r'; // carriage return
const char LF = '\n'; // line feed
const char terminating_null = '\0';

/// adjust the field pointers by an offset
template <bool = build_record> // by default call the specialized case, below
void UpdateFieldOffsets(ElegentFileReader::Record&, const size_t)
{}

template <> // if building a record
void UpdateFieldOffsets<build_record>(ElegentFileReader::Record& record, const size_t pointer_diff)
{
    for (auto& field : record)
        field -= pointer_diff;
}

const char unicode_signature[] = {'\xEF', '\xBB', '\xBF'};
const size_t unicode_signature_length = sizeof(unicode_signature) / sizeof(*unicode_signature);
const char* const unicode_signature_end = unicode_signature + unicode_signature_length;

bool UnicodeSignatureContains(const char c)
{
    return std::find(unicode_signature, unicode_signature_end, c) != unicode_signature_end;
}
}

ElegentFileReader::ElegentFileReader(
    const uint32_t read_size, const char delimiter, const bool as_one, const size_t max_fields)
    : file_size_(0),
      read_size_(read_size),
      pos_(0),
      eof_(true),
      pos_of_buffer_in_file_(0),
      pos_of_record_in_buffer_(0),
      last_actual_character_(0),
      delimiter_(delimiter),
      multiple_delimiters_as_one_(as_one),
      max_fields_requested_(max_fields)
{
    ENSURE(read_size, >, 0);
    ENSURE(!UnicodeSignatureContains(delimiter));
}

/// free all memory and call the constructor
/// done this way to preserve const-ness
void __vectorcall ElegentFileReader::close()
{
    if (ifs_.is_open())
        ifs_.close();
    if (!ifs_)
        throw std::exception("cannot close file");

    // reset everything in case we want to open another file
    Buffer().swap(buffer_);
    Record().swap(record_);
    new(this) ElegentFileReader(read_size_, delimiter_, multiple_delimiters_as_one_, max_fields_requested_);
}

/// open the file for reading
void __vectorcall ElegentFileReader::open(const char* const filename)
{
    ENSURE(!isOpen());

    ifs_.open(std::filesystem::u8path(filename), std::ios::binary);

    if (!ifs_ || !isOpen()) {
        ifs_.clear();
        throw std::exception((std::string("cannot open ") + filename).c_str());
    }

    file_size_ = std::filesystem::file_size(filename);
    eof_ = ftell() == file_size_;
}

/// remember the end-of-record position by restoring the character that was replaced with a terminating_null back into the buffer
void __vectorcall ElegentFileReader::restoreEolCharacter()
{
    switch (last_actual_character_) {
    case LF:
        buffer_[pos_ - 1] = LF;
        break;
    case CR:
        buffer_[pos_ - 2] = CR; // part of the CRLF combination (the LF would have remained intact)
        break;
    default: ;
    }
    last_actual_character_ = terminating_null;
}

/// check for delimiter only - 
///             once we have reached max_fields_requested_, 
///             delimters don't count as delimiters anymore (just ignore them all to end of line)
bool __vectorcall ElegentFileReader::foundDelimiter() const
{
    return delimiter_ == last_actual_character_ && record_.size() < max_fields_requested_;
}

/// same as above, but also check for delimiters that have been replaced with terminating_null
bool __vectorcall ElegentFileReader::foundDelimiterOr_0() const
{
    return terminating_null == last_actual_character_ || foundDelimiter();
}

/// check if there is a partial record at the end of the buffer
/// if there is, that partial record will be copied to the front; so adjust pos_ accordingly
size_t __vectorcall ElegentFileReader::getPartialRecordLength_andAdjustPos()
{
    // buffer_ will be empty after a seek(); seek() would have set pos_; pos_ must be retained
    if (buffer_.empty())
        return 0;

    // adjust pos_ to cater for moving the partial record to the front
    if (pos_ < buffer_.size())
        pos_ -= pos_of_record_in_buffer_;
    else
        pos_ = 0; // reached end of buffer; no partial record

    // return the length of the partial record
    return buffer_.size() - pos_of_record_in_buffer_;
}


template<bool> // IfBuildingRecord -: store it in the buffer
void __vectorcall ElegentFileReader::storeTerminatingNull()
{
    buffer_[pos_] = terminating_null;
}

template<> // not BuildingRecord  -: store it as last_actual_character_ 
void __vectorcall ElegentFileReader::storeTerminatingNull<!build_record>()
{  // no need to store it in the buffer; and by storing it in the last_actual_character_, we won't restore eol when starting the next record
    last_actual_character_ = terminating_null;
}

/// check for end of record and increment pos_ to look at the next character
/// cater for a line that ends at the end of the file, lines ending with LF or lines ending with CRLF
/// update pos_ if returning true (because in these cases pos_ may be incremented by 0, 1 or 2)
template<bool IfBuildingRecord>
bool __vectorcall ElegentFileReader::isEndOfRecord()
{
    // remember the actual character: delimiters and end of line characters are replaced by terminating_null
    if (isEOF()) {
        // the last record in the file ended without a LF or a CRLF 
        last_actual_character_ = terminating_null;
        // pos_ stays the same
        return true;
    }
    last_actual_character_ = buffer_[pos_];

    // check for CRLF 1st  - more common in our environment
    // in case of '\r, we have to check if the next char (i.e., buffer_[pos_ + 1]) is LF 
    //   but if the file ends with CR, then buffer_[pos_ + 1] will be out of range of the vector
    //   we can still safely check that memory location, because we added a terminating_null just beyond the end of the buffer
    CHECK(pos_ < buffer_.size() || *(buffer_.data() + pos_ + 1) == terminating_null);  // check the assertion above

    if (CR == last_actual_character_ && LF == *(buffer_.data() + pos_ + 1)) {
        storeTerminatingNull<IfBuildingRecord>();
        pos_ += 2; // skip over both the CR and the LF
        eof_ = ftell() == file_size_;
        return true;
    }

    // also check for line ending with LF only
    if (LF == last_actual_character_) {
        storeTerminatingNull<IfBuildingRecord>();
        ++pos_;
        eof_ = ftell() == file_size_;
        return true;
    }

    return false; // we didn't reach end of line; don't update pos_ - the caller will do it
}

template <>
void __vectorcall ElegentFileReader::replaceDelimiterWith_0<build_record>()
{
    // replace the delimiter with terminating_null so that the preceding char* (the field) is null terminated
    if (foundDelimiter())
        buffer_[pos_] = terminating_null;
}

/// if we're still within the current record, increase pos_ and return true
/// else return false to break out of the loop in the calling function
template <bool IfBuildingRecord>
bool __vectorcall ElegentFileReader::moveToNextChar()
{
    if (isEndOfRecord<IfBuildingRecord>()) // will update pos_ appropriately
        return false;

    replaceDelimiterWith_0<IfBuildingRecord>(); // this function will use current pos_ (only increment pos_afterwards - i.e. next line)

    ++pos_;
    eof_ = ftell() == file_size_;
    return true;
}

/// check for a partial record at the end of the buffer
/// if found, move it to the front, adjust field pointers and update pos_of_buffer_in_file_
template <bool IfBuildingRecord>
size_t __vectorcall ElegentFileReader::handlePartialRecord()
{
    const size_t partial_record_length = getPartialRecordLength_andAdjustPos();

    if (0 == partial_record_length) { // no partial record: we're starting a brand new buffer
        pos_of_buffer_in_file_ += buffer_.size();

    } else if (pos_of_record_in_buffer_ > 0) { // there is a partial record and it is not the entire buffer_
        // copy the partial record to the front of the buffer
        std::memcpy(&buffer_[0], &buffer_[pos_of_record_in_buffer_], partial_record_length);

        // we've moved the characters in the buffer to the front
        // now update those field pointers in the record to reflect their new offsets
        UpdateFieldOffsets<IfBuildingRecord>(record_, pos_of_record_in_buffer_);

        pos_of_buffer_in_file_ += buffer_.size() - partial_record_length;
    }

    pos_of_record_in_buffer_ = 0;
    buffer_.resize(partial_record_length); // resize the buffer smaller
    return partial_record_length;
}

/// if we're at the beginnig of the file, check for the unicode signature and skip over it 
/// returns true if we found skipped it, false otherwise
bool __vectorcall ElegentFileReader::skippedOverUnicodeSignature()
{
    if (0 == pos_of_buffer_in_file_  // this is the 1st buffer/block in the file
        && pos_ < unicode_signature_length && buffer_.size() >= unicode_signature_length
        && !memcmp(unicode_signature, buffer_.data(), unicode_signature_length)) {
        pos_ = unicode_signature_length;
        eof_ = ftell() == file_size_;
        return true;
    }
    return false;
}

template <>
void __vectorcall ElegentFileReader::resizeBuffer<build_record>(const size_t new_size)
{
    // resizing bigger might re-allocate memory: remember the old data pointer to adjust the fields in the record
    const auto original_buffer_data = buffer_.data();
    resizeBuffer(new_size);

    // adjust all the word/field pointers found thus far. if buffer memory was re-allocated,
    // we've got the original buffer begin, so we can safely adjust the pointers :)
    if (original_buffer_data != buffer_.data())
        UpdateFieldOffsets(record_, original_buffer_data - buffer_.data());
}

template <>
void __vectorcall ElegentFileReader::clearRecordStartedInUnicodeSignature<build_record>()
{
    // special case if read_size_ is really small (i.e. < unicode_signature_length): if we found and skipped over the unicode signature, 
    // empty out the record we've found - it is the signature characters and not really the record
    if (!record_.empty()) {
        // just clear out the record - it will restart on the next character
        CHECK(record_.size(), == , 1);
        CHECK(read_size_, < , unicode_signature_length);
        record_.clear();
        last_actual_character_ = terminating_null;
    }
}


/// read a block from the file into the buffer; we always read on "read_size" boundaries
/// make sure the buffer area is big enough to hold the newly read data
template <bool IfBuildingRecord>
void __vectorcall ElegentFileReader::readBlock()
{
    const size_t partial_record_length = handlePartialRecord<IfBuildingRecord>();
    // resize the buffer; big enough for everything (+1 for a terminating_null)
    resizeBuffer<IfBuildingRecord>(partial_record_length + read_size_ + 1);

    uint32_t bytes_read = read_size_;
    if (!ifs_.read(buffer_.data() + partial_record_length, read_size_)) {
        if (ifs_.rdstate() != (std::ios::failbit | std::ios::eofbit))
            throw std::exception("error on read");
        ifs_.clear();
        bytes_read = static_cast<uint32_t>(ifs_.gcount());
    }

    // in case fewer bytes were read than requested (reached eof), resize buffer to reflect the correct size
    const size_t new_size = partial_record_length + bytes_read;
    buffer_[new_size] = terminating_null;
    buffer_.resize(new_size);

    if (skippedOverUnicodeSignature())
        // TODO: when read_size_ gets changed to a template parameter, call the next function if read_size_ < unicode_signature_length
        clearRecordStartedInUnicodeSignature<IfBuildingRecord>();
}


/// seek to a block boundary position; pos will be set appropriately whithin the block
void __vectorcall ElegentFileReader::seek(const uint64_t filepos_requested)
{
    ENSURE(filepos_requested, <= , file_size_);

    restoreEolCharacter();

    uint64_t new_buffer_pos = filepos_requested / read_size_ * read_size_;  // quotient * read_size_
    pos_ = static_cast<size_t>(filepos_requested - new_buffer_pos);  // remainder
    eof_ = filepos_requested == file_size_;

    // check if (filepos_wanted is in the current buffer) ---> no actual reading required
    if (new_buffer_pos == pos_of_buffer_in_file_) {
        // skip over unicode signature if at the front of the file, 
        // but whether we do or not doesn't matter - no need to check return value
        skippedOverUnicodeSignature();

        // and we don't have to clear out the record here: the buffer is still intact, the record will be too
        return;
    }

    // clearout the buffers and position the file for the next read
    buffer_.clear();
    record_.clear();
    pos_of_buffer_in_file_ = new_buffer_pos;

    if (!ifs_.seekg(new_buffer_pos))
        throw std::exception("error on seek");;

}

/// add the field pointers to the Record
template <>
void __vectorcall ElegentFileReader::checkForNewField<build_record>()
{
    // check if we have a new field in the buffer: either we have a delimiter, or one that was replaced by a terminating_null, or this is the 1st char in a new line
    if (foundDelimiterOr_0() || record_.empty()) {
        // pos_ is pointing to the next char in the buffer; if we treat multiple delimiters as one, 
        //          then, if this char is also a delimter (or a terminating_null), don't add it to the record
        if (!(multiple_delimiters_as_one_ && (delimiter_ == buffer_[pos_] || terminating_null == buffer_[pos_])))
            record_.push_back(&buffer_[pos_]);
    }
}

/// the common code for readRecord() and skipRecord(), distinguished by the template paramneter IfBuildingRecord
template <bool IfBuildingRecord>
const ElegentFileReader::Record& __vectorcall ElegentFileReader::getNextRecord()
{
    record_.clear();
    restoreEolCharacter();
    pos_of_record_in_buffer_ = pos_;

    // if we're at eof, leave the record_ empty to signal eof
    if (!isEOF())
        do {
            // fill the buffer if it is getting empty
            while (pos_ + 1 >= buffer_.size() // leave 1 char to check for CRLF
                && ftell() + 1 < file_size_) // and there are more blocks to be read
                readBlock<IfBuildingRecord>();

            checkForNewField<IfBuildingRecord>();

        } while (moveToNextChar<IfBuildingRecord>());

    return record_; // in case of skipRecord, the record will be empty and this result unused
}

void __vectorcall ElegentFileReader::skipRecord()
{
    getNextRecord<!build_record>();
}

const ElegentFileReader::Record& __vectorcall ElegentFileReader::readRecord()
{
    return getNextRecord<build_record>();
}


