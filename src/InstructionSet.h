#ifndef INSTRUCTIONSET_H
#define INSTRUCTIONSET_H

#include "shared/Basic.h"
#include <vector>
#include <string>

// Design Pattern: Visitor.
class Visitor {
    public:
        virtual std::string compile_exit(int value) { return ""; };
};

// Instruction set.
class Instr {
    public:
        virtual ~Instr() = default;

        // TODO: make this a pure virtual method.
        virtual std::string compile(Visitor &v) { 
            crash("If you see this message it means that there is an error inside Parser.\n");
            return ""; 
        };
};

class Exit : public Instr {
    public:
        explicit Exit(int exit_value) : exit_value(exit_value) {}

        std::string compile(Visitor &v) { return v.compile_exit(this->exit_value); }

        int exit_value;
};

#endif // INSTRUCTIONSET_H
