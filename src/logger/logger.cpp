/*
** Definition of Logger
** Should be treaded as singleton
*/

#include "logger.hpp"

namespace Logger {

// Logger should be ready before any other classes or variables
std::unique_ptr<Logger> __attribute__((init_priority(1000))) pLOG = std::make_unique<Logger>("Asm");

}
