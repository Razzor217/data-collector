/**
 * @file collector.h
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Definition of a data collector
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */


#include "collector.h"

#include <chrono>
#include <iostream>

#include <errno.h>
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>


void Collector::monitor_and_collect()
{
    std::cout << "Start monitoring " << input_path << std::endl;
    std::cout << "Please type <q> and press <RETURN> to stop the program and quit." << std::endl;

    monitor_thread = std::thread{&Collector::monitor, this};
    collector_thread = std::thread{&Collector::collect, this};

    pollfd poll_descriptor {};
    poll_descriptor.fd = STDIN_FILENO;
    poll_descriptor.events = POLLIN;

    char buffer {};

    for (;;)
    {
        int poll_number {poll(&poll_descriptor, 1, -1)};

        if (poll_number < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::cerr << "Error while polling from stdin" << std::endl;
            break;
        }

        if (poll_number > 0)
        {
            if (poll_descriptor.revents & POLLIN)
            {
                /*
                 * read from stdin until newline character is found,
                 * then quit out of loop
                 */
                while (read(STDIN_FILENO, &buffer, 1) > 0 && buffer != 'q')
                {
                    continue;
                }
                break;
            }
        }
    }

    std::cout << "Stopping worker threads" << std::endl;

    // request interruption of worker threads
    is_running.store(false);

    monitor_thread.join();
    collector_thread.join();
}


void Collector::monitor()
{
    int file_descriptor {inotify_init1(IN_NONBLOCK)};

    if (file_descriptor < 0)
    {
        std::cerr << "Error while initializing inotify."
                  << "Please type <q> and press <RETURN> to quit"
                  << std::endl;
        return;
    }

    int watch_descriptor {inotify_add_watch(file_descriptor, input_path.c_str(), IN_CREATE)};

    if (watch_descriptor < 0)
    {
        std::cerr << "Error, cannot watch " << input_path.c_str() << "."
                  << "Please type <q> and press <RETURN> to quit"
                  << std::endl;
        return;
    }

    pollfd poll_descriptor {};
    poll_descriptor.fd = file_descriptor;
    poll_descriptor.events = POLLIN;

    while (is_running.load())
    {
        int poll_number {poll(&poll_descriptor, 1, 30)};

        if (poll_number < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::cerr << "Error while polling from stdin" << std::endl;
            break;
        }

        if (poll_number > 0)
        {
            if (poll_descriptor.revents & POLLIN)
            {
                handle_file_event(file_descriptor);
            }
        }
    }

    std::cout << "Stop monitoring " << input_path << std::endl;

    inotify_rm_watch(file_descriptor, watch_descriptor);
    close(file_descriptor);

    std::cout << "Monitor thread finished" << std::endl;
}


void Collector::collect()
{
    while (is_running.load())
    {
        if (!queue->empty())
        {
            std::filesystem::path const file {queue->pop()};

            std::vector<std::filesystem::path> file_names
            {
                collect_files(file.parent_path(), selection)
            };
            std::vector<std::filesystem::path> temporaries;

            collect_disk_usage(file_names, temporaries, file.parent_path());

            std::string hash {std::to_string(std::hash<std::string>{}(std::string{file.filename().c_str()}))};
            store_files
            (
                file_names,
                temporaries,
                output_path / std::filesystem::path {"archive." + hash + ".tar"}
            );
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    std::cout << "Collector thread finished" << std::endl;
}


Collector::Collector
(
    std::filesystem::path const& input_path,
    std::filesystem::path const& output_path,
    FileSelection const selection
) :
    input_path {input_path},
    output_path {output_path},
    selection {selection}
{
}


void Collector::set_regex(std::regex const& regex)
{
    file_regex = regex;
}


std::vector<std::filesystem::path> Collector::collect_files
(
    std::filesystem::path const& path,
    FileSelection const selection
)
{
    std::cout << "Collecting selected files from " << path << std::endl;

    switch (selection)
    {
        case FileSelection::FILES:
        {
            std::vector<std::filesystem::path> files;
            for (auto const& entry : std::filesystem::directory_iterator{path})
            {
                if (entry.is_regular_file())
                {
                    files.push_back(entry.path());
                }
            }
            return files;
        }
        case FileSelection::FILES_AND_DIRECTORIES:
        default:
        {
            std::vector<std::filesystem::path> files;
            for (auto const& entry : std::filesystem::recursive_directory_iterator{path})
            {
                files.push_back(entry.path());
            }
            return files;
        }
    }
}


void Collector::collect_disk_usage
(
    std::vector<std::filesystem::path>& files,
    std::vector<std::filesystem::path>& temporaries,
    std::filesystem::path const& output_path
)
{
    std::filesystem::path usage {output_path / std::filesystem::path {"disk_usage.txt"}};

    std::cout << "Writing disk usage information to " << usage << std::endl;

    for (auto const& file : files)
    {
        std::system(std::string {"du -sh " + file.native() + " >> " + usage.native()}.c_str());
    }

    files.push_back(usage);
    temporaries.push_back(usage);
}


void Collector::store_files
(
    std::vector<std::filesystem::path> const& files,
    std::vector<std::filesystem::path> const& temporaries,
    std::filesystem::path const& output_file,
    bool delete_temporaries
)
{
    std::cout << "Storing collected data as tar archive in " << output_file << std::endl;

    std::string command {"tar -cf " + output_file.native()};

    for (auto const& file : files)
    {
        command += " " + file.native();
    }

    std::system(command.c_str());

    if (delete_temporaries)
    {
        // remove temporary files
        for (auto const& file : temporaries)
        {
            std::system(std::string {"rm -rf " + file.native()}.c_str());
        }
    }
}


void Collector::handle_file_event(int const file_descriptor)
{
    alignas(inotify_event) char buffer[BUFFER_SIZE];

    while (is_running.load())
    {
        int n = read(file_descriptor, buffer, BUFFER_SIZE);

        if (n < 0 && errno != EAGAIN)
        {
            std::cerr << "Error while reading from " << input_path.c_str() << "."
                  << "Please type <q> and press <RETURN> to quit"
                  << std::endl;
            return;
        }

        if (n <= 0)
        {
            break;
        }

        inotify_event const* event {};
        for (char* ptr {buffer}; ptr < buffer + n; ptr += sizeof(inotify_event) + event->len)
        {
            event = (inotify_event const*) ptr;
            if (event->len)
            {
                if ((event->mask & IN_CREATE) && std::regex_match(event->name, file_regex))
                {
                    std::cout << "New matching file/directory '" << event->name << "' created" << std::endl;
                    queue->push(input_path / std::filesystem::path{event->name});
                }
            }
        }
    }
}
