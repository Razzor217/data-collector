/**
 * @file correctness.cpp
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Unit tests to test correctness of data collection
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */


#include <gtest/gtest.h>

#include "../collector.h"

#include <filesystem>
#include <fstream>
#include <regex>


constexpr std::string_view REGEX {"core\\.[a-zA-Z]+(\\.[a-f0-9]+)+\\.lz4"};


TEST(RegexTest, DefaultTest)
{
    std::string file {"core.Service.0.lz4"};
    EXPECT_TRUE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, CoreTest1)
{
    std::string file {"c.Service.0.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, CoreTest2)
{
    std::string file {"CORE.Service.0.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, CoreTest3)
{
    std::string file {".Service.0.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}


TEST(RegexTest, IdentifierTest1)
{
    std::string file {"core.aAzZ.0.lz4"};
    EXPECT_TRUE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, IdentifierTest2)
{
    std::string file {"core.ServiceName0123.0.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, IdentifierTest3)
{
    std::string file {"core..0.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, Lz4Test1)
{
    std::string file {"core.Service.0.lz"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, Lz4Test2)
{
    std::string file {"core.Service.0.LZ4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, Lz4Test3)
{
    std::string file {"core.Service.0."};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, DotsTest1)
{
    std::string file {"coreService.0.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, DotsTest2)
{
    std::string file {"coreService0lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, DotsTest3)
{
    std::string file {"core.Service.0lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, HexGroupTest1)
{
    std::string file {"core.Service.g.lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, HexGroupTest2)
{
    std::string file {"core.Service..lz4"};
    EXPECT_FALSE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, HexGroupTest3)
{
    std::string file {"core.Service.0.0.lz4"};
    EXPECT_TRUE(std::regex_match(file, std::regex {std::string {REGEX}}));
}

TEST(RegexTest, HexGroupTest4)
{
    std::string file {"core.Service.0.0.0.0.0.0.0.0.0.0.lz4"};
    EXPECT_TRUE(std::regex_match(file, std::regex {std::string {REGEX}}));
}



TEST(FileCollectionTest, FilesTest1)
{
    namespace fs = std::filesystem;

    fs::create_directory("sandbox");
    std::ofstream{"sandbox/file"};

    auto const files = Collector::collect_files(fs::path {"sandbox"}, FileSelection::FILES);

    EXPECT_EQ(files.front(), fs::path {"sandbox/file"});

    fs::remove_all("sandbox");
}

TEST(FileCollectionTest, FilesTest2)
{
    namespace fs = std::filesystem;

    fs::create_directories("sandbox/dir");
    std::ofstream{"sandbox/file"};

    auto const files = Collector::collect_files(fs::path {"sandbox"}, FileSelection::FILES);

    EXPECT_EQ(files.front(), fs::path {"sandbox/file"});
    EXPECT_NE(files.front(), fs::path {"sandbox/dir"});

    fs::remove_all("sandbox");
}

TEST(FileCollectionTest, FilesAndDirectoriesTest1)
{
    namespace fs = std::filesystem;

    fs::create_directories("sandbox/dir");

    auto const files = Collector::collect_files(fs::path {"sandbox"}, FileSelection::FILES_AND_DIRECTORIES);

    EXPECT_EQ(files.front(), fs::path {"sandbox/dir"});

    fs::remove_all("sandbox");
}

TEST(FileCollectionTest, FilesAndDirectoriesTest2)
{
    namespace fs = std::filesystem;

    fs::create_directories("sandbox/dir/dir/file");

    auto const files = Collector::collect_files(fs::path {"sandbox"}, FileSelection::FILES_AND_DIRECTORIES);

    EXPECT_NE(std::find(files.begin(), files.end(), fs::path {"sandbox/dir"}), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), fs::path {"sandbox/dir/dir"}), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), fs::path {"sandbox/dir/dir/file"}), files.end());

    fs::remove_all("sandbox");
}

TEST(DiskUsageTest, DiskUsageTest1)
{
    namespace fs = std::filesystem;

    fs::create_directory("sandbox");
    fs::create_directory("sandbox_output");
    std::ofstream {"sandbox/file"};

    std::vector<fs::path> files {fs::path {"sandbox/file"}};
    std::vector<fs::path> temporaries {};

    Collector::collect_disk_usage(files, temporaries, fs::path {"sandbox_output"});

    EXPECT_TRUE(fs::exists(fs::path {"sandbox_output/disk_usage.txt"}));

    fs::remove_all("sandbox");
    fs::remove_all("sandbox_output");
}

TEST(ArchiveTest, ArchiveTest1)
{
    namespace fs = std::filesystem;

    fs::create_directory("sandbox");
    fs::create_directory("sandbox_output");
    std::ofstream {"sandbox/file"};

    std::vector<fs::path> files {fs::path {"sandbox/file"}};
    std::vector<fs::path> temporaries {};

    Collector::store_files(files, temporaries, fs::path {"sandbox_output/archive"}, false);

    EXPECT_TRUE(fs::exists(fs::path {"sandbox_output/archive"}));

    fs::remove_all("sandbox");
    fs::remove_all("sandbox_output");
}

TEST(ArchiveTest, ArchiveTest2)
{
    namespace fs = std::filesystem;

    fs::create_directory("sandbox");
    fs::create_directory("sandbox_output");
    std::ofstream {"sandbox/file"};
    std::ofstream {"sandbox/temp"};

    std::vector<fs::path> files {fs::path {"sandbox/file"}};
    std::vector<fs::path> temporaries {fs::path {"sandbox/temp"}};

    Collector::store_files(files, temporaries, fs::path {"sandbox_output/archive.tar"}, true);

    EXPECT_TRUE(fs::exists(fs::path {"sandbox_output/archive.tar"}));
    EXPECT_FALSE(fs::exists(fs::path {"sandbox/temp"}));

    fs::remove_all("sandbox");
    fs::remove_all("sandbox_output");
}

TEST(ArchiveTest, ArchiveTest3)
{
    namespace fs = std::filesystem;

    fs::create_directory("sandbox");
    fs::create_directory("sandbox_output");

    std::system("echo \"hello\" > sandbox/file");

    std::vector<fs::path> files {fs::path {"sandbox/file"}};
    std::vector<fs::path> temporaries {};

    Collector::store_files(files, temporaries, fs::path {"sandbox_output/archive.tar"}, false);

    std::system("cd sandbox_output && tar -xf archive.tar");

    EXPECT_TRUE(fs::exists(fs::path {"sandbox_output/sandbox/file"}));

    fs::remove_all("sandbox");
    fs::remove_all("sandbox_output");
}
