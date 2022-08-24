/**
 * @file collector.h
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Declaration of a data collector
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */


#pragma once


#include "fifo.h"


#include <atomic>
#include <filesystem>
#include <functional>
#include <regex>
#include <thread>
#include <vector>



constexpr int BUFFER_SIZE = 1024;
constexpr int CACHE_LINE_SIZE = 64;


enum class FileSelection
{
    FILES,
    FILES_AND_DIRECTORIES
};


class Collector
{
public:
    void monitor_and_collect();

    void monitor();

    void collect();

    Collector
    (
        std::filesystem::path const& input_path,
        std::filesystem::path const& output_path,
        FileSelection const selection
    );

    void set_regex(std::regex const& regex);


    static std::vector<std::filesystem::path> collect_files
    (
        std::filesystem::path const& path,
        FileSelection const selection
    );

    static void collect_disk_usage
    (
        std::vector<std::filesystem::path>& files,
        std::vector<std::filesystem::path>& temporaries,
        std::filesystem::path const& output_path
    );

    static void store_files
    (
        std::vector<std::filesystem::path> const& files,
        std::vector<std::filesystem::path> const& temporaries,
        std::filesystem::path const& output_file,
        bool delete_temporaries = true
    );


private:
    void handle_file_event(int const file_descriptor, int const watch_descriptor);

private:
    std::filesystem::path input_path {};
    std::filesystem::path output_path {};
    FileSelection selection;

    std::regex file_regex {"core\\.[a-zA-Z]+(\\.[a-f0-9]+)+\\.lz4"};

    std::thread monitor_thread {};
    std::thread collector_thread {};

    fifo_ptr<std::filesystem::path> queue
    {
        std::make_unique<blocking_fifo<std::filesystem::path>>()
    };

    alignas(CACHE_LINE_SIZE) std::atomic<bool> is_running {true};

};
