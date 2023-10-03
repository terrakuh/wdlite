#pragma once

#if defined(WDLITE_ENABLE_LOGGING)
#include <iostream>

#define WDLITE_INFO(log) std::cout << "INFO: " << log << "\n"
#define WDLITE_DEBUG(log) std::cout << "DEBUG: " << log << "\n"
#else
#define WDLITE_INFO(log) static_cast<void>(0)
#define WDLITE_DEBUG(log) static_cast<void>(0)
#endif
