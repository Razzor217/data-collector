/**
 * @file main.cpp
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Execution entry point for data collector
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */


#include "collector.h"

#include <iostream>


void print_usage(std::string const& name)
{
    std::cout << "Usage: " << name
        << " INPUT_PATH OUTPUT_PATH ( -f | -d )"
        << std::endl;
}


int main(int argc, char** argv)
{
    if (argc != 4)
    {
        print_usage(std::string{argv[0]});
        return -1;
    }

    FileSelection selection;

    if (strcmp(argv[3], "-f") == 0)
    {
        selection = FileSelection::FILES;
    }
    else if (strcmp(argv[3], "-d") == 0)
    {
        selection = FileSelection::FILES_AND_DIRECTORIES;
    }
    else
    {
        print_usage(std::string{argv[0]});
        return -1;
    }

    Collector c
    {
        std::filesystem::path {argv[1]},
        std::filesystem::path {argv[2]},
        selection
    };

    c.monitor_and_collect();

    return 0;
}
