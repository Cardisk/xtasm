#include "Token.h"

#include "../shared/Basic.h"

std::string ttype_str(TokenType type) {
    // handling all the types.
    static_assert(TokenType::COUNT == 7, "ERROR: ttype_str doesnt handle all the possible tokens!\n");

    switch (type) {
        case TokenType::INVALID: return "INVALID";
        case TokenType::CODE: return "CODE";
        case TokenType::DATA: return "DATA";
        case TokenType::VAR: return "VAR";
        case TokenType::QMARK: return "QMARK";
        case TokenType::EXIT: return "EXIT";
        case TokenType::INT: return "INT";
        // case TokenType::DEC: return "DEC";
        default:
            crash("`ttype_str` unreachable branch. This could be a bug into the Lexer.");
            return "";
    }
}

std::string token_loc(Token &tkn) {
    // calling the helper function with the correct fields.
    return loc(tkn.file, tkn.line, tkn.column);
}
