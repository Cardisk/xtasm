#ifndef INSTRUCTIONSET_H
#define INSTRUCTIONSET_H

#include "shared/Basic.h"
#include <memory>
#include <vector>
#include <string>

// Forward declaration for the Visitor class.
class Instr;

// Design Pattern: Visitor.
class Visitor {
    public:
        // TODO: make these pure virtual methods.
        virtual std::string compile_data(std::vector<std::unique_ptr<Instr>> variables) { return ""; } 
        virtual std::string compile_code(std::vector<std::unique_ptr<Instr>> instructions) { return ""; } 
        virtual std::string compile_exit(std::string value) { return ""; }
        virtual std::string compile_var(std::string name, std::string value) { return ""; }
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

class Data : public Instr {
    public:
        explicit Data(std::vector<std::unique_ptr<Instr>> variables) : variables(std::move(variables)) {} 

        std::string compile(Visitor &v) { return v.compile_data(std::move(this->variables)); }

        std::vector<std::unique_ptr<Instr>> variables;
};

class Code : public Instr {
    public:
        explicit Code(std::vector<std::unique_ptr<Instr>> instructions) : instructions(std::move(instructions)) {} 

        std::string compile(Visitor &v) { return v.compile_code(std::move(this->instructions)); }

        std::vector<std::unique_ptr<Instr>> instructions;
};

class Exit : public Instr {
    public:
        explicit Exit(std::string exit_value) : exit_value(exit_value) {}

        std::string compile(Visitor &v) { return v.compile_exit(this->exit_value); }

        std::string exit_value;
};

class Var : public Instr {
    public:
        explicit Var(std::string name, std::string value) : name(name), value(value) {}

        std::string compile(Visitor &v) { return v.compile_var(this->name, this->value); }

        std::string name;
        std::string value;
};

#endif // INSTRUCTIONSET_H
