#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>

#include "../shared/Basic.h"
#include "../shared/Option.h"
#include "Token.h"
#include "../InstructionSet.h"

class Parser {
    public:
        // Default c'tor.
        explicit Parser() = default;

        // Used to parse the tokens into instructions.
        std::vector<std::unique_ptr<Instr>> parse_tkns(std::vector<Token> tkns);
    private:
        // Used to peek the next token.
        Option<Token> peek(uint_t offset = 0);
        // Used to advance and retrive the current token.
        Option<Token> advance();
        // Used to obtain the next instruction.
        std::unique_ptr<Instr> next();
        // Used to parse the #data section.
        std::unique_ptr<Data> parse_data();
        // Used to parse the #code section.
        std::unique_ptr<Code> parse_code();
        // Used to parse an exit instruction.
        std::unique_ptr<Exit> parse_exit();
        // Used to parse a variable declaration.
        std::unique_ptr<Var> parse_variable();

        // The program represented as tokens.
        std::vector<Token> tkns;
        // Current token inside the vector.
        uint_t cursor;
};

#endif // PARSER_H
