/**
 * @file component.cpp
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Component tests for data collection
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */


#include <gtest/gtest.h>

#include "../collector.h"

#include <fstream>


class ComponentTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        namespace fs = std::filesystem;

        fs::create_directory("sandbox");
        fs::create_directory("sandbox_output");

        collector = std::make_unique<Collector>
        (
            fs::path {"sandbox"},
            fs::path {"sandbox_output"},
            FileSelection::FILES
        );

        worker = std::thread {&ComponentTest::run, this};
    }

    virtual void TearDown()
    {
        worker.join();

        namespace fs = std::filesystem;

        fs::remove_all("sandbox");
        fs::remove_all("sandbox_output");
    }

    void run()
    {
        collector->monitor_and_collect();
    }

protected:
    std::unique_ptr<Collector> collector {nullptr};
    std::thread worker;
};


// TEST_F(ComponentTest, DataCollectionTest1)
// {
//     char* buffer = {"q\n"};
//     write(STDIN_FILENO, buffer, 2);
// }


TEST_F(ComponentTest, DataCollectionTest2)
{
    std::system("touch sandbox/core.service.0.lz4");
    std::string hash {std::to_string(std::hash<std::string>{}("core.service.0.lz4"))};

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(3s);

    EXPECT_TRUE(std::filesystem::exists(std::filesystem::path {"sandbox_output/archive." + hash + ".tar"}));

    // char* buffer = {"q\n"};
    // write(STDIN_FILENO, buffer, 2);
}
