#pragma once

#include <atomic>
#include <algorithm>
#include <functional>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

/*struct input_observer
{
    notify()
};*/

struct input_listener // : public input_observable
{
    explicit input_listener(int fd) : tty_fd(fd) { listener_thread = std::thread(&input_listener::input_loop, this, tty_fd); }
    ~input_listener() { is_running = false; listener_thread.join(); }

    void add_callback(char key, std::function<void(void)> callback)
    {
//        std::cout << __PRETTY_FUNCTION__ << "\n";
        // TODO: add mutex when key bindings become mutable after thread start
        if (callback_map.find(key) == callback_map.end())
        {
            callback_map[key] = callback;
        }
    }

    std::map<char, std::function<void(void)>> callback_map;

    std::thread listener_thread;
    std::atomic<bool> is_running { true };

    int tty_fd { 0 };

    void input_loop(int tty_fd)
    {
        fd_set keys_fd;
        
        while (is_running)
        {
            FD_ZERO(&keys_fd);
            FD_SET(tty_fd, &keys_fd);

            auto r_select = select(tty_fd + 1, &keys_fd, NULL, NULL, NULL);

            char keys[8];
            std::fill(keys, keys + 8, '\0');

            auto r_read = read(tty_fd, keys, 8);

            if (r_read >= 1)
            {
                std::string kb_input(keys);
                if (std::find(kb_input.begin(), kb_input.end(), 'q') != kb_input.end())
                    is_running = false;

                auto cb = callback_map.find(keys[0]);
                if (cb != callback_map.end())
                {
                    cb->second();
                }
            }
        }
    }

};
