#ifndef TOOL_CPP_MACRO_PRINT
#define TOOL_CPP_MACRO_PRINT

#define PRINT_INFO std::cout << tool0::time_stomp() << " [INFO] " << __FILE__ << "(" << __FUNCTION__ << ":" << __LINE__ << ") >> "
#define PRINT_ERROR std::cout << tool0::time_stomp() << " [ERROR] " << __FILE__ << "(" << __FUNCTION__ << ":" << __LINE__ << ") >> "

#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace tool0 {
    __time64_t timenow;
    std::_Timeobj<char, const tm*> time_stomp();
}

#endif