#ifndef BASIC_H
#define BASIC_H

#include <string>

// 'Basic.h' contains all the useful stuff that I think I will
// reuse all the time.

// Shorthand for unsigned long int.
typedef unsigned long int uint_t;

// Helper function to craft a location.
// Actually used by:
//  - Src
//  - Token
std::string loc(std::string &filepath, uint_t line, uint_t column);

// Helper function to crash the program and report the error.
// Parameters:
// - const char[] aka "..."
void crash(std::string &&msg);

// Helper function to crash the program and report the error.
// Parameters:
// - std::string & aka variables
void crash(std::string &msg);

// Helper function to log debug messages
// Parameters:
// - std::string & aka variables
void log(std::string &msg);

// Helper function to log debug messages
// Parameters:
// - std::string & aka "..."
void log(std::string &&msg);

#endif // BASIC_H
