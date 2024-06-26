#include "Token.h"

#include "../shared/Basic.h"

std::string ttype_str(TokenType type) {
    // handling all the types.
    static_assert(TokenType::COUNT == 33, "ERROR: ttype_str doesnt handle all the possible tokens!\n");

    switch (type) {
        case TokenType::INVALID  : return "INVALID";
        case TokenType::CODE     : return "CODE";
        case TokenType::DATA     : return "DATA";
        case TokenType::REG      : return "REG";
        case TokenType::WHILE    : return "WHILE";
        case TokenType::FOR      : return "FOR";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::LOOP     : return "LOOP";
        case TokenType::BREAK    : return "BREAK";
        case TokenType::LABEL    : return "LABEL";
        case TokenType::IF       : return "IF";
        case TokenType::IN       : return "IN";
        case TokenType::ELSE     : return "ELSE";
        case TokenType::END      : return "END";
        case TokenType::EQ       : return "EQ";
        case TokenType::NEQ      : return "NEQ";
        case TokenType::GRT      : return "GRT";
        case TokenType::GEQ      : return "GEQ";
        case TokenType::LT       : return "LT";
        case TokenType::LEQ      : return "LEQ";
        case TokenType::AND      : return "AND";
        case TokenType::OR       : return "OR";
        case TokenType::EXIT     : return "EXIT";
        case TokenType::ADD      : return "ADD";
        case TokenType::SUB      : return "SUB";
        case TokenType::MUL      : return "MUL";
        case TokenType::MOV      : return "MOV";
        case TokenType::JMP      : return "JMP";
        case TokenType::NAME     : return "NAME";
        case TokenType::ENUM     : return "ENUM";
        case TokenType::VAR      : return "VAR";
        case TokenType::QMARK    : return "QMARK";
        case TokenType::INT      : return "INT";
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
