#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace tool0 {
    //timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());

    std::_Timeobj<char, const tm*> time_stomp() {
        timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
        const __time64_t temp_const = timenow;
        return std::put_time(std::localtime(&temp_const), "%y-%m-%d %OH:%OM:%OS");
    }
}
