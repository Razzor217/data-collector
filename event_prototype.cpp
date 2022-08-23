/**
 * @file event_prototype.cpp
 * @author Max Beddies (max dot beddies at t dash online dot de)
 * @brief Prototype for file creation event triggers
 * @version 0.1
 * @date 2022-08-23
 *
 * @copyright Copyright (c) 2022
 *
 */


#include <iostream>

#include <sys/inotify.h>
#include <unistd.h>


#define BUFFER_SIZE 1024


int main(int argc, char** argv)
{
    if (argc != 2 )
    {
        std::cout << "Usage: " << argv[0] << " INPUT_PATH" << std::endl;
        return -1;
    }

    int file_descriptor {inotify_init()};

    if (file_descriptor < 0)
    {
        std::cerr << "initialization failed" << std::endl;
        return -1;
    }

    int watch_descriptor {inotify_add_watch(file_descriptor, argv[1], IN_CREATE)};

    if (watch_descriptor < 0)
    {
        std::cerr << "cannot watch " << argv[1] << std::endl;
        return -1;
    }

    alignas(inotify_event) char buffer[BUFFER_SIZE];
    int const n {read(file_descriptor, buffer, BUFFER_SIZE)};

    inotify_event const* event {};
    for (char* ptr {buffer}; ptr < buffer + n; ptr += sizeof(inotify_event) + event->len)
    {
        event = (inotify_event const*) ptr;
        if (event->len)
        {
            if (event->mask & IN_CREATE)
            {
                std::cout << "new file/directory '" << event->name << "' created" << std::endl;
            }
        }
    }


    inotify_rm_watch(file_descriptor, watch_descriptor);
    close(file_descriptor);

    return 0;
}
