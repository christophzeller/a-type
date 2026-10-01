#include "world/engine.hxx"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

int main()
{
    std::srand(std::time({}));
    engine the_game {};

    the_game.run();
    
    return 0;
}
