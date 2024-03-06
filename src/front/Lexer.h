#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>

#include "../shared/Basic.h"
#include "../shared/Option.h"
#include "Token.h"

class Lexer {
    public:
        // Default c'tor.
        explicit Lexer() {}
        // Used to tokenize a file.
        std::vector<Token> lex_file(std::string filepath);
    private:
        // Used to craft a token from the current state.
        Token token();
        // Used to peek the next character.
        Option<char> peek(uint_t offset = 0);
        // Used to advance the cursor.
        Option<char> advance(); 
        // Used to get the next Token.
        Option<Token> next();
        // Used to reset the Lexer state.
        void reset();

        // filepath.
        std::string filepath;
        // source code.
        std::string src;
        // current line.
        uint_t line;
        // used to discover new tokens.
        uint_t cursor;
        // current token column.
        uint_t old_cursor;
};

#endif // LEXER_H
