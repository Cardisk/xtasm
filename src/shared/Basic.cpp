#include "Basic.h"

#include <cstdlib>
#include <string>

#include "Logger.h"

std::string loc(std::string &filepath, uint_t line, uint_t column) {
    // crafts a location like <filepath>:<line>:<column>.
    return filepath + ":" + std::to_string(line) + ":" + std::to_string(column);
}

void crash(std::string &msg) {
    // getting the shared Logger.
    Logger &logger = Logger::get_logger();
    // printing the msg.
    logger.log(Logger::ERR, msg);
    // crashing the application.
    ::exit(1);
}

void crash(std::string &&msg) {
    crash(msg);
}

void log(std::string &msg) {
    // getting the shared logger.
    Logger &logger = Logger::get_logger();
    // printing the msg.
    logger.log(Logger::DBG, msg);
}

void log(std::string &&msg) {
    log(msg);
}
