#include "pch.h"

#include <chrono>

#include "CSVFileIndex.h"
#include "ElegentFileReader.h"
#include "Ensure.h"

TEST(ElegentFileReader, NonExistentFile) {
    ENSURE(1, <, 2);

    EXPECT_TRUE(10);
    EXPECT_NO_THROW(ENSURE(10)); // ENSURE with 1 param checks for a logical true
    EXPECT_ANY_THROW(ENSURE_TRUE(10)); // ENSURE_TRUE checks for an actual true (a 1)

    ElegentFileReader efr;
    EXPECT_NO_THROW(efr.seek(0));
    EXPECT_NO_THROW(efr.close());
    EXPECT_ANY_THROW(efr.open("file that does not exist"));
    EXPECT_NO_THROW(efr.seek(0));
    EXPECT_ANY_THROW(efr.seek(1));
}

TEST(ElegentFileReader, EmptyFile) {
    ElegentFileReader efr;
    EXPECT_NO_THROW(efr.open("TestFiles\\empty file.txt"));
    EXPECT_EQ(0, efr.ftell());
    EXPECT_TRUE(efr.isEOF());

    EXPECT_NO_THROW(efr.seek(0));
    EXPECT_EQ(0, efr.ftell());
    EXPECT_TRUE(efr.isEOF());

    EXPECT_ANY_THROW(efr.seek(1));
}

TEST(ElegentFileReader, SmallFile) {
    ElegentFileReader efr;
    EXPECT_NO_THROW(efr.open("TestFiles\\fr_record.tbl"));
    EXPECT_EQ(0, efr.ftell());
    EXPECT_FALSE(efr.isEOF());

    EXPECT_NO_THROW(efr.seek(0));
    EXPECT_EQ(0, efr.ftell());
    EXPECT_FALSE(efr.isEOF());

    EXPECT_NO_THROW(efr.seek(1));
    EXPECT_EQ(1, efr.ftell());

    efr.readBlock<nullptr>();
    EXPECT_FALSE(efr.isEOF());
    EXPECT_NO_THROW(efr.seek(239));
    EXPECT_EQ(239, efr.ftell());

    EXPECT_TRUE(efr.isEOF());
}

TEST(ElegentFileReader, SkipThroughSmallFile) {
    ElegentFileReader efr;
    EXPECT_NO_THROW(efr.open("TestFiles\\fr_record.tbl"));
    size_t count = 0;
    while(!efr.isEOF()) {
        efr.skipRecord();
        ++count;
    }

    EXPECT_EQ(7, count);
}

TEST(ElegentFileReader, ReadThroughSmallFile) {
    const char* expected[][7] = {
        { "Product", "Group", "var1", "var2", "var3", "var4", "var5"},
        {"default", "abc", "1.1", "1.2", "1.3", "1.4", "1.5"},
        {"default", "bce", "2.1", "2.2", "2.3", "2.4", "2.5"},
        {"default", "cef", "3.1", "3.2", "3.3", "3.4", "3.5"},
        {"default", "efg", "4.1", "4.2", "4.3", "4.4", "4.5"},
        {"default", "fgh", "5.1", "5.2", "5.3", "5.4", "5.5"},
        {"default", "ghi", "6.1", "6.2", "6.3", "6.4", "6.5"}
    };

    for (uint32_t read_size = 1; read_size < 250; ++read_size) {
        ElegentFileReader efr(read_size);
        EXPECT_NO_THROW(efr.open("TestFiles\\fr_records.tbl"));

        size_t count = 0;
        while (!efr.isEOF()) {
            auto rec = efr.readRecord();
            EXPECT_EQ(7, rec.size());
            for (size_t ri = 0; ri < 7; ++ri)
                EXPECT_STREQ(expected[count][ri], rec[ri]);
            ++count;
        }

        EXPECT_EQ(7, count);
    }
}



TEST(ElegentFileReader, ReadThroughSmallFileLF) {
    const char* expected[][7] = {
        {""},
        {"Product", "Group", "var1", "var2", "var3", "var4", "var5"},
        {"default", "abc", "1.1", "1.2", "1.3", "1.4", "1.5"},
        {"default", "bce", "2.1", "2.2", "2.3", "2.4", "2.5"},
        {"\rdefault", "cef", "3.1", "3.2", "3.3", "3.4", "3.5"},
        {"default", "efg", "4.1", "4.2", "4.3", "4.4", "4.5"},
        {"default", "fgh", "5.1", "5.2\r", "5.3", "5.4", "5.5"},
        {"default", "ghi", "6.1", "6.2", "6.3", "6.4", "6.5\r"}
    };

    for (uint32_t read_size = 1; read_size < 250; ++read_size) {
        ElegentFileReader efr(read_size);
        EXPECT_NO_THROW(efr.open("TestFiles\\fr_records_lf.tbl"));

        size_t count = 0;
        while (!efr.isEOF()) {
            auto rec = efr.readRecord();
            EXPECT_EQ(count ? 7 : 1, rec.size());
            for (size_t ri = 0; ri < rec.size(); ++ri)
                EXPECT_STREQ(expected[count][ri], rec[ri]);
            ++count;
        }

        EXPECT_EQ(8, count);
    }
}


TEST(ElegentFileReader, SeekSmallFile) {
    const char* expected[][7] = {
        { "Product", "Group", "var1", "var2", "var3", "var4", "var5"},
        {"default", "abc", "1.1", "1.2", "1.3", "1.4", "1.5"},
        {"default", "bce", "2.1", "2.2", "2.3", "2.4", "2.5"},
        {"default", "cef", "3.1", "3.2", "3.3", "3.4", "3.5"},
        {"default", "efg", "4.1", "4.2", "4.3", "4.4", "4.5"},
        {"default", "fgh", "5.1", "5.2", "5.3", "5.4", "5.5"},
        {"default", "ghi", "6.1", "6.2", "6.3", "6.4", "6.5"}
    };

    for (uint32_t read_size = 1; read_size < 250; ++read_size) {
        //std::cout << read_size << std::endl;
        ElegentFileReader efr(read_size);
        EXPECT_NO_THROW(efr.open("TestFiles\\fr_records.tbl"));

        size_t count = 2;
        efr.seek(75);
        efr.skipRecord();
        while (!efr.isEOF()) {
            auto rec = efr.readRecord();
            EXPECT_EQ(7, rec.size());
            for (size_t ri = 0; ri < rec.size(); ++ri)
                EXPECT_STREQ(expected[count][ri], rec[ri]);
            ++count;
        }

        EXPECT_EQ(7, count);
    }
}

TEST(ElegentFileReader, SeekMoreSmallFile) {
    const char* expected[][7] = {
        { "Product", "Group", "var1", "var2", "var3", "var4", "var5"},
        {"default", "abc", "1.1", "1.2", "1.3", "1.4", "1.5"},
        {"default", "bce", "2.1", "2.2", "2.3", "2.4", "2.5"},
        {"default", "cef", "3.1", "3.2", "3.3", "3.4", "3.5"},
        {"default", "efg", "4.1", "4.2", "4.3", "4.4", "4.5"},
        {"default", "fgh", "5.1", "5.2", "5.3", "5.4", "5.5"},
        {"default", "ghi", "6.1", "6.2", "6.3", "6.4", "6.5"}
    };

    for (uint32_t read_size = 41; read_size < 250; ++read_size) {
        //std::cout << read_size << std::endl;
        ElegentFileReader efr(read_size);
        EXPECT_NO_THROW(efr.open("TestFiles\\fr_records.tbl"));

        size_t count = 4;
        bool jumped_back = false;
        efr.seek(130);
        efr.skipRecord();
        while (!efr.isEOF()) {
            auto rec = efr.readRecord();
            EXPECT_EQ(7, rec.size());
            for (size_t ri = 0; ri < rec.size(); ++ri)
                EXPECT_STREQ(expected[count][ri], rec[ri]);
            if (5 == count && !jumped_back) {
                jumped_back = true;
                efr.seek(50);
                efr.skipRecord();
                count = 1;
            }
            ++count;
        }

        EXPECT_EQ(7, count);
    }
}


using FileReader = ElegentFileReader;
using Record = FileReader::Record;

/// Test base FileReader features
class FileReaderTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

protected:
    static std::string JoinRecordFromArray(const Record& rec);
    FileReader fr_;
};

std::string FileReaderTestFixture::JoinRecordFromArray(const Record& record) {
    std::string s;
    for (const char* field : record) {
        s += std::string(field);
        s += ' ';
    }
    if (!s.empty())
        s.erase(s.size() - 1);
    return s;
}

/// Combine private interface tests to not flood friends declarations
TEST(FileReaderTest, TestPrivateInterface)
{
    // ConstructorDefault
    {
        ASSERT_NO_THROW(FileReader fr;);
    }

    // Check initialization values
    {
        const FileReader fr;
        EXPECT_EQ(4_Kb, fr.read_size_);
        EXPECT_FALSE(fr.isOpen());
        EXPECT_EQ(0, fr.pos_);
    }

    // OpenSuccessfully
    {
        FileReader fr;
        ASSERT_NO_THROW(fr.open("TestFiles\\FileReader\\read_only_file.txt"));
        ASSERT_TRUE(fr.isOpen());
    }

    // close
    {
        FileReader fr;
        ASSERT_NO_THROW(fr.open("TestFiles\\FileReader\\block16.txt"));
        fr.close();
        EXPECT_FALSE(fr.isOpen());
    }

    // Dtor
    {
        {
            FileReader fr;
            EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\block16.txt"));
        } // dtor should close the handle
        FileReader fr;
        EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\block16.txt"));
        EXPECT_TRUE(fr.isOpen());
    }

    // ReadBlockFromBuffer
    //{
    //    FileReader fr;
    //    const uint32_t block_size_ = fr.read_size_;
    //    uint32_t size = (block_size_ << 2);
    //    const uint32_t tail = (size / 7);
    //    size += tail;
    //    std::unique_ptr<char[]> upc(new char[size]);
    //    char* buf = upc.get();
    //    for (size_t i = 0; i < 4; i++) {
    //        EXPECT_TRUE(fr.readBlockFromBuffer(buf, block_size_));
    //        buf += block_size_;
    //    }

    //    EXPECT_FALSE(fr.readBlockFromBuffer(buf, tail));
    //}

    //// ReadAllFile
    //{
    //    FileReader fr;
    //    fr.set_block_size(16);
    //    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\block16.txt"));
    //    while (fr.readBlock())
    //        ;
    //}

//    // ERROR_INVALID_NANDLE failure
//    {
//        FileReader fr;
//        fr.open("TestFiles\\FileReader\\block16.txt");
//
//        const HANDLE hFile = fr.file_handle_; // save original handle to restore it later to be able to close the file
//
//        // modify object handle via reference to incorrect value
//        size_t& fh = reinterpret_cast<size_t&>(fr.file_handle_); // assuming HANDLE is PVOID
//        fh = reinterpret_cast<size_t>(INVALID_HANDLE_VALUE) - 1; // force to fail with expected ERROR_INVALID_NANDLE Windows error code
//
//        EXPECT_THROW(fr.readBlock(), int);
//        try {
//            fr.readBlock();
//        }
//        catch (int& ex) {
//            //EXPECT_STREQ("FileReader: Cannot read file.\nWindows API function ReadFile failed. Last error code 6.", ex.what()); // 6 - ERROR_INVALID_NANDLE
//        }
//        fr.file_handle_ = hFile; // restore handle - in other case SEH in ::CloseHandle()
//    }
//
//} // End of TestPrivateInterface
//
//TEST_F(FileReaderTestFixture, set_block_size)
//{
//    EXPECT_NO_THROW(fr_.set_block_size(64));
//    EXPECT_EQ(64, fr_.get_block_size());
//}
//
//TEST_F(FileReaderTestFixture, ReadFail)
//{
//    EXPECT_THROW(fr_.open("not_existing_file"), int);
//    try {
//        fr_.open("not_existing_file");
//    }
//    catch (int& ex) {
//        //        EXPECT_STREQ("FileReader: Cannot open a file \"not_existing_file\".\nWindows API function CreateFile failed. Last error code 2.", ex.what());
//    }
}

// Disable tests with exceptions
TEST_F(FileReaderTestFixture, openTwice)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\read_only_file.txt"));
    EXPECT_ANY_THROW(fr_.open("TestFiles\\FileReader\\read_only_file.txt"));

}

//TEST_F(FileReaderTestFixture, ReadRecordNotopen)
//{
//    EXPECT_FALSE(fr_.isOpen());
//    EXPECT_ANY_THROW(fr_.readRecord());
//}
// End of disable tests with exceptions

// TODO: write a test to re-read data in the same block after they were separated by 0 in the first read

TEST_F(FileReaderTestFixture, ReadRecordOneBlock)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_record_one_block.tbl"));

    // TODO: test each line values
    Record record = fr_.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr_.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr_.readRecord();
    EXPECT_FALSE(record.empty());
}



TEST(FileReaderTest, ReadRecords)
{
    FileReader fr(16u);
    //fr.set_block_size(76);

    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_records.tbl"));

    // TODO: test each line values
    Record record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
}

TEST(FileReaderTest, ReadRecords_crlf_crosses_block_boundary)
{
    FileReader fr(75u);
    //fr.set_block_size(75);

    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_records.tbl"));

    // TODO: test each line values
    Record record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
}


TEST_F(FileReaderTestFixture, SkipRecordOneBlock)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_record_one_block.tbl"));

    // collecting records start positions
    uint64_t idx = fr_.ftell();
    fr_.skipRecord();

    // line 0 - header
    EXPECT_EQ(0, idx);  // expecting BOM

    // line 1
    idx = fr_.ftell();
    fr_.skipRecord();
    EXPECT_EQ(43, idx);

    // line 2
    idx = fr_.ftell();
    fr_.skipRecord();
    EXPECT_EQ(76, idx);

    // after the line 2, eof
    idx = fr_.ftell();
    EXPECT_TRUE(fr_.isEOF());
    EXPECT_NO_THROW(fr_.skipRecord());
    Record rec;
    EXPECT_NO_THROW(rec = fr_.readRecord());
    EXPECT_TRUE(rec.empty());
    EXPECT_TRUE(fr_.isEOF());

    fr_.seek(76);
    rec = fr_.readRecord();
    EXPECT_EQ(7, rec.size());
}

TEST_F(FileReaderTestFixture, ReadRecordTwoBlocks)
{
    FileReader fr(128u);
    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_record.tbl"));
    //fr.set_block_size(128);
    //fr.set_max_record_size(32);

    // TODO: test each line values
    // line 1
    Record record = fr.readRecord();
    EXPECT_FALSE(record.empty());
    EXPECT_STREQ("Product Group var1 var2 var3 var4 var5", JoinRecordFromArray(record).c_str());

    // line 2
    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
    EXPECT_STREQ("default abc 1.1 1.2 1.3 1.4 1.5", JoinRecordFromArray(record).c_str());

    // line 3
    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
    EXPECT_STREQ("default abc 2.1 2.2 2.3 2.4 2.5", JoinRecordFromArray(record).c_str());

    // block ends in the middle of 4th line - "default    abc    3.1    3.2"
    record = fr.readRecord();
    EXPECT_FALSE(record.empty());

    std::string line = JoinRecordFromArray(record);
    EXPECT_STREQ("default abc 3.1 3.2 3.3 3.4 3.5", line.c_str());

    EXPECT_EQ(std::string("default abc 3.1 3.2 3.3 3.4 3.5"), line.c_str()); // alternative string compare

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
    EXPECT_STREQ("default abc 4.1 4.2 4.3 4.4 4.5", JoinRecordFromArray(record).c_str());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
    EXPECT_STREQ("default abc 5.1 5.2 5.3 5.4 5.5", JoinRecordFromArray(record).c_str());

    record = fr.readRecord();
    EXPECT_FALSE(record.empty());
    EXPECT_STREQ("default abc 6.1 6.2 6.3 6.4 6.5", JoinRecordFromArray(record).c_str());
}

TEST_F(FileReaderTestFixture, SkipRecordTwoBlocks)
{
    FileReader fr(128u);
    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_record.tbl"));/*
    fr.set_block_size(128);
    fr.set_max_record_size(32);*/

    uint64_t idx = fr.ftell();
    fr.skipRecord(); // skip header
    EXPECT_EQ(0, idx);

    std::vector<uint64_t> file_index;
    while (!fr.isEOF()) {
        idx = fr.ftell();
        fr.skipRecord();
        file_index.push_back(idx);
    }

    EXPECT_EQ(6, file_index.size());

    // line 0 is a header

    // line 1
    EXPECT_EQ(43, file_index[0]);

    // line 2
    EXPECT_EQ(76, file_index[1]);

    // line 3
    EXPECT_EQ(109, file_index[2]);

    // line 4
    EXPECT_EQ(142, file_index[3]);

    // line 5
    EXPECT_EQ(175, file_index[4]);

    // line 6
    EXPECT_EQ(208, file_index[5]);

}

// Testing on larger datasets to detect the case when the block is finished exactly on the end of the line
// Test3 includes Test1 and Test2 (the same data file). First two test are aimed to simplify debugging.
TEST_F(FileReaderTestFixture, SkipRecordIdxTest1)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_skip_test1.tbl"));

    std::vector<uint64_t> file_index;
    while (!fr_.isEOF()) {
        uint64_t idx = fr_.ftell();
        fr_.skipRecord();
        file_index.push_back(idx);
    }

    EXPECT_EQ(128, file_index.size());

    EXPECT_EQ(0, file_index[0]); // BOM
    EXPECT_EQ(730, file_index[1]);
    EXPECT_EQ(5677, file_index[15]);
    EXPECT_EQ(43972, file_index[122]);
    EXPECT_EQ(45755, file_index[127]);
}


// Larger dataset 2.
TEST_F(FileReaderTestFixture, SkipRecordIdxTest2)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_skip_test2.tbl"));

    std::vector<uint64_t> file_index;
    while (!fr_.isEOF()) {
        uint64_t idx = fr_.ftell();
        fr_.skipRecord();
        file_index.push_back(idx);
    }

    EXPECT_EQ(1200, file_index.size());

    EXPECT_EQ(0, file_index[0]);
    EXPECT_EQ(730, file_index[1]);
    EXPECT_EQ(5677, file_index[15]);
    EXPECT_EQ(43972, file_index[122]);
    EXPECT_EQ(160133, file_index[448]);
    EXPECT_EQ(280130, file_index[786]);
    EXPECT_EQ(359992, file_index[1011]);
    EXPECT_EQ(426656, file_index[1199]);
}

// Larger dataset 3.
TEST_F(FileReaderTestFixture, SkipRecordIdxTest3)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_skip_test3.tbl"));

    std::vector<uint64_t> file_index;
    while (!fr_.isEOF()) {
        uint64_t idx = fr_.ftell();
        fr_.skipRecord();
        file_index.push_back(idx);
    }

    EXPECT_EQ(4000, file_index.size());

    // These should still match
    EXPECT_EQ(0, file_index[0]);
    EXPECT_EQ(730, file_index[1]);
    EXPECT_EQ(280130, file_index[786]);
    EXPECT_EQ(426656, file_index[1199]);
    EXPECT_EQ(427001, file_index[1200]);
    EXPECT_EQ(494616, file_index[1391]);
    EXPECT_EQ(723981, file_index[2048]);
    EXPECT_EQ(1057030, file_index[3000]);
    EXPECT_EQ(1190389, file_index[3379]);
    EXPECT_EQ(1406313, file_index[3999]);
}

/// File positioning
TEST_F(FileReaderTestFixture, GetCurrentPosFail)
{
    // file not open - return 0
    EXPECT_EQ(0, fr_.ftell());
}

TEST_F(FileReaderTestFixture, GetCurrentPos)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_record.tbl"));
    EXPECT_NO_THROW(fr_.ftell());
    EXPECT_EQ(0, fr_.ftell());
    //fr_.set_block_size(43);

    fr_.readRecord();
    const uint64_t cur_pos = fr_.ftell();
    EXPECT_EQ(43, cur_pos);

    fr_.skipRecord();
    EXPECT_EQ(76, fr_.ftell());

    fr_.skipRecord();
    EXPECT_EQ(109, fr_.ftell());

    fr_.skipRecord(); // moving over the block border
    EXPECT_EQ(142, fr_.ftell());

    fr_.skipRecord();
    EXPECT_EQ(175, fr_.ftell());

}

TEST_F(FileReaderTestFixture, SkipPosEqualGetRecordPos)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_record.tbl"));
    fr_.readRecord();
    uint64_t get_record_pos = fr_.ftell();
    fr_.close();

    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_record.tbl"));
    fr_.skipRecord();
    const uint64_t skip_record_pos = fr_.ftell();
    EXPECT_EQ(get_record_pos, skip_record_pos);
}

//TEST_F(FileReaderTestFixture, SetCurrentPosFail)
//{
//    // file not open
//    EXPECT_ANY_THROW(fr_.seek(0));
//    try {
//        fr_.seek(1);
//    }
//    catch (int& ex) {
//        //        EXPECT_STREQ("FileReader::set_current_pos: cannot set current file position.\nWindows API function SetFilePointer failed. Last error code 6.", ex.what());
//    }
//}

TEST_F(FileReaderTestFixture, SetCurrentPos)
{
    EXPECT_NO_THROW(fr_.open("TestFiles\\FileReader\\fr_record.tbl"));
    fr_.readRecord();
    uint64_t pos = fr_.ftell();

    fr_.skipRecord();
    fr_.seek(0);

    fr_.readRecord();
    const uint64_t after_move_pos = fr_.ftell();
    EXPECT_EQ(pos, after_move_pos);
}

//TEST_F(FileReaderTestFixture, CurrentSymbol)
//{
//    // need to open file first
//    EXPECT_EQ(0, fr_.get_current_sym());
//
//    fr_.open("TestFiles\\FileReader\\fr_record.tbl");
//    EXPECT_EQ(0, fr_.get_current_sym());
//
//    fr_.read_first_block();
//    EXPECT_EQ('P', fr_.get_current_sym());
//
//    fr_.skipRecord();
//    EXPECT_EQ('d', fr_.get_current_sym());
//}

TEST_F(FileReaderTestFixture, RecordEndsOnTheBlockBoundary)
{
    FileReader fr(48u);
    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_block.tbl"));
    //fr.set_block_size(48); // ends exactly on the record end with '\r\n' (1st record - consider 3 for BOM)
    //fr.set_max_record_size(48);

    // tests condition in FileReader.cpp, parse_next_record(), ln 326: if (pos_ < block_size_ - 1)
    // - avoid skip_eol() and readBlock() at the end of parsing the record
    Record record = fr.readRecord();
    EXPECT_STREQ("default abc 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8", JoinRecordFromArray(record).c_str());

    record = fr.readRecord();
    EXPECT_STREQ("default bcd 2.1 2.2 2.3 2.4 2.5 2.6 2.7 2.8", JoinRecordFromArray(record).c_str());
}

TEST_F(FileReaderTestFixture, WordEndsOnTheBlockBoundary)
{
    FileReader fr(64u);
    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_block.tbl"));
    //fr.set_block_size(64);
    //fr.set_max_record_size(48); // 32 is small enough

    Record record = fr.readRecord();
    EXPECT_STREQ("default abc 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8", JoinRecordFromArray(record).c_str());

    // 2.1\t in line 2 is the end of the block record (indicies 61,62,63). add_word() and parse_next_record() should behave correctly
    record = fr.readRecord();
    EXPECT_STREQ("default bcd 2.1 2.2 2.3 2.4 2.5 2.6 2.7 2.8", JoinRecordFromArray(record).c_str());
}

TEST_F(FileReaderTestFixture, MaxRecordSizeLowerRecordSize)
{
    FileReader fr(24u);
    EXPECT_NO_THROW(fr.open("TestFiles\\FileReader\\fr_block.tbl"));
    //fr.set_block_size(24);
    //fr.set_max_record_size(32); // 2nd record is 43

    Record r = fr.readRecord();
    EXPECT_NO_THROW(r = fr.readRecord());
    EXPECT_LT(0u, r.size());
}

// Separate test as need different parameters for ctor.
// A case for parsing config. Use '*' as * is a config comment line.
#ifdef _DEBUG
TEST(FileReaderTest, ConsecutiveDelimetersAsOne)
#else
TEST(FileReaderTest, ConsecutiveDelimetersAsOne)
#endif
{
    FileReader fr(50, ' ', true, 3); // small block to test block borders
    fr.open("TestFiles\\FileReader\\delimieters_as_one.tbl");

    auto line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("ef    ghi   jkl  *", line[2]);

    fr.seek(0);
    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("ef    ghi   jkl  *", line[2]);


    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("a", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("ef    ghi   jkl  *", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("def", line[1]);
    EXPECT_STREQ("ghi   jkl  *", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("ef    ghi   jkl  *", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("a", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("ef    ghi   jkl  *", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("def", line[1]);
    EXPECT_STREQ("ghi   jkl  *", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("de", line[1]);
    EXPECT_STREQ("fg hi   jkl  *", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("a", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("efghijkl", line[2]);

    line = fr.readRecord();
    EXPECT_EQ(3, line.size());
    EXPECT_STREQ("abc", line[0]);
    EXPECT_STREQ("d", line[1]);
    EXPECT_STREQ("ef    ghi   jkl  *", line[2]);

} // Test ConsecutiveDelimetersAsOne

TEST(CSVFileIndex, EmptyFile) {
    ElegentFileReader efr;
    EXPECT_NO_THROW(efr.open("TestFiles\\empty file.txt"));

    CSVFileIndex index;
    index.createIndex(efr, 0);

    EXPECT_TRUE(index.catalogue_.empty());
}

TEST(CSVFileIndex, SmallFile) {
    ElegentFileReader efr;
    EXPECT_NO_THROW(efr.open("TestFiles\\fr_record.tbl"));

    CSVFileIndex index0;
    index0.createIndex(efr, 0);
    EXPECT_EQ(1U, index0.catalogue_.size());

    CSVFileIndex index2;
    index2.createIndex(efr, 2);
    EXPECT_EQ(6U, index2.catalogue_.size());
}

TEST(CSVFileIndex, LargeFile) {

    ElegentFileReader efr(64_Kb, ',', false);
    EXPECT_NO_THROW(efr.open("TestFiles\\Main model_Main Model - MCEV~control~cf.csv"));

    CSVFileIndex index2;

    typedef std::chrono::high_resolution_clock clock;
    auto start_time = clock::now();
    index2.createIndex(efr, 2);
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start_time);
    std::cout << time.count() << " total time\n";

    EXPECT_EQ(492U, index2.catalogue_.size());
}


TEST(CSVFileIndex, ReadLargeFile) {

    ElegentFileReader efr(64_Kb, ',', false);
    EXPECT_NO_THROW(efr.open("TestFiles\\Main model_Main Model - MCEV~control~cf.csv"));

    CSVFileIndex index2;

    typedef std::chrono::high_resolution_clock clock;
    const auto start_time = clock::now();

    std::vector<uint64_t> record_positions;

    while (!efr.isEOF()) {
        record_positions.push_back(efr.ftell());
        efr.skipRecord();
    }
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start_time);
    std::cout << time.count() << " total time\n";

    EXPECT_EQ(590893, record_positions.size());
}

TEST(CSVFileIndex, Find_in_LargeFile) {
    ElegentFileReader efr(16_Kb, ',', false, 4);
    EXPECT_NO_THROW(efr.open("TestFiles\\Main model_Main Model - MCEV~control~cf.csv"));

    CSVFileIndex index2;
    index2.createIndex(efr, 2);

    EXPECT_EQ(0, index2.find("abc").record_count_);
    auto index = index2.find("batch0|ANN_4750_YYY_RP_Other");
    EXPECT_EQ(1201, index.record_count_);
}
