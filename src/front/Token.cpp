#include "Token.h"

#include "../shared/Basic.h"

std::string ttype_str(TokenType type) {
    // handling all the types.
    static_assert(TokenType::COUNT == 20, "ERROR: ttype_str doesnt handle all the possible tokens!\n");

    switch (type) {
        case TokenType::INVALID: return "INVALID";
        case TokenType::CODE: return "CODE";
        case TokenType::DATA: return "DATA";
        case TokenType::REG: return "REG";
        case TokenType::LABEL: return "LABEL";
        case TokenType::IF: return "IF";
        case TokenType::IN: return "IN";
        case TokenType::ELSE: return "ELSE";
        case TokenType::END: return "END";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::EXIT: return "EXIT";
        case TokenType::ADD: return "ADD";
        case TokenType::SUB: return "SUB";
        case TokenType::MOV: return "MOV";
        case TokenType::NAME: return "NAME";
        case TokenType::ENUM: return "ENUM";
        case TokenType::VAR: return "VAR";
        case TokenType::QMARK: return "QMARK";
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
