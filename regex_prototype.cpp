/**
 * @file regex_prototype.cpp
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Prototype for regex matching
 * @version 0.1
 * @date 2022-08-23
 *
 * @copyright Copyright (c) 2022
 *
 */


#include <functional>
#include <iostream>
#include <regex>
#include <string>


int main()
{
    std::regex const file_regex {"core\\.[a-zA-Z]+(\\.[a-f0-9]+)+\\.lz4"};
    std::string const test_string {"core.ServiceName.3057.57dd721409bc4ab4b38a3c33a36a608a.3717.1647975805000000.lz4"};


    if (std::regex_match(test_string, file_regex))
    {
        std::cout << test_string << " matches." << std::endl;
        std::cout << "Hash of " << test_string << ": " << std::hash<std::string>{}(test_string) << std::endl;
    }
    else
    {
        std::cout << test_string << " does not match." << std::endl;
    }

    return 0;
}