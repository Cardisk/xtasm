#ifndef TOKEN_H
#define TOKEN_H

#include <string>

#include "../shared/Basic.h"

// All the possible token types.
enum TokenType {
    INVALID,
    CODE,
    DATA,
    EXIT,
    VAR,
    QMARK,
    INT,
    // DEC,
    COUNT,
};

// Used to get a human-readable-name of the TokenType.
std::string ttype_str(TokenType type);

// Struct representing a token with its position inside
// the file system.
struct Token {
    // type of the token.
    TokenType type;
    // text of the token.
    std::string text;
    // filepath of the token.
    std::string file;
    // line where the token is located.
    uint_t line;
    // column where the token is located.
    uint_t column;
};

// Used to craft a stringified location of the token
std::string token_loc(Token &tkn);

#endif // TOKEN_H
