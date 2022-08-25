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


/**
 * @brief Select which files to collect
 *
 */
enum class FileSelection
{
    /**
     * @brief Select regular files only, do NOT recurse into subdirectories
     *
     */
    FILES,

    /**
     * @brief Select regular files as well as directories, recursively
     *
     */
    FILES_AND_DIRECTORIES
};


/**
 * @brief Class controlling the monitoring and data collection within a given
 * directory.
 *
 * The collector class starts and stops two worker threads.
 * One worker is tasked with monitoring the input directory for file creation
 * events while the other worker collects data upon arrival of file creation
 * events.
 * The collected data is then stored as tar archive in the output directory.
 *
 */
class Collector
{
public:
    /**
     * @brief Monitor the input directory for file creation events and collect
     * data upon event trigger.
     *
     * The name of the created file has to match the `file_regex`.
     * This method starts and stops the two worker threads.
     *
     */
    void monitor_and_collect();

    /**
     * @brief Monitor file creation events in the input directory
     *
     */
    void monitor();

    /**
     * @brief Upon arrival of file creation events, collect data and store it
     * in the output directory
     *
     */
    void collect();

    /**
     * @brief Construct a new Collector object
     *
     * @param input_path Directory to monitor
     * @param output_path Directory to store collected data in
     * @param selection Data collection mode
     */
    Collector
    (
        std::filesystem::path const& input_path,
        std::filesystem::path const& output_path,
        FileSelection const selection
    );

    /**
     * @brief Configure the regex used for matching file names
     *
     * @param regex Regex matched against during file creation events
     */
    void set_regex(std::regex const& regex);

    /**
     * @brief Collect files in a specified directory, depending on the selection
     * mode
     *
     * @see FileSelection
     *
     * @param path Directory to collect files from
     * @param selection File selection mode
     * @return Resulting list of file paths
     */
    static std::vector<std::filesystem::path> collect_files
    (
        std::filesystem::path const& path,
        FileSelection const selection
    );

    /**
     * @brief Collect disk usage information from a given list of files
     *
     * @param files Files to collect disk usage information from
     * @param temporaries List of temporary files
     * @param output_path Output directory for disk usage information
     */
    static void collect_disk_usage
    (
        std::vector<std::filesystem::path>& files,
        std::vector<std::filesystem::path>& temporaries,
        std::filesystem::path const& output_path
    );

    /**
     * @brief Store a given list of files as a tar archive
     *
     * @param files Files to store in the archive
     * @param temporaries Temporary files to delete
     * @param output_file  Given file path of the archive
     * @param delete_temporaries Determines whether temporaries are deleted
     */
    static void store_files
    (
        std::vector<std::filesystem::path> const& files,
        std::vector<std::filesystem::path> const& temporaries,
        std::filesystem::path const& output_file,
        bool delete_temporaries = true
    );


private:
    void handle_file_event(int const file_descriptor);

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
