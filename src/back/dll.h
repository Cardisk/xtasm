#ifndef DLL_H
#define DLL_H

#include <vector>
#include <string>

#include "../InstructionSet.h"

// Compile functions used to translate the syntax tree into target code.
std::string compile(std::string handle, std::vector<std::unique_ptr<Instr>> &instructions);

#endif // DLL_H
