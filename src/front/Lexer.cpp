#include "Lexer.h"

#include <fstream>
#include <stdexcept>
#include <cerrno>

#include "Token.h"

std::vector<Token> Lexer::lex_file(std::string filepath) {
    // reset the Lexer state.
    this->reset();
    this->filepath = filepath;

    std::ifstream file;
    file.open(filepath);

    if (!file.is_open()) {
        auto msg = "Unable to open '" + filepath + "'. " + std::strerror(errno);
        crash(msg);
    } 

    std::string line;
    while (file.good()) {
        std::getline(file, line);    
        this->src += line + "\n";
    }
    file.close();
    
    std::vector<Token> tkns;

    // FIXME: this will NOT tokenize correctly.
    while (true) {
        auto tkn = this->next();
        if (tkn.is_none()) break;

        tkns.push_back(tkn.unwrap());
    }

    return tkns;
}

Token Lexer::token() {
    // old_cursor points to the start of the token
    // cursor points to the char after the token
    auto text_len = this->cursor - this->old_cursor;
    auto text = this->src.substr(this->old_cursor, text_len);
    
    Token tkn = (Token) {
        .type = TokenType::INVALID,
        .text = text,
        .file = this->filepath,
        .line = this->line,
        .column = this->old_cursor,
    };

    this->old_cursor = this->cursor;

    return tkn;
}

Option<char> Lexer::peek(uint_t offset) {
    try {
        return Option<char>::some(
                this->src.at(this->cursor + offset)
        );
    } catch (std::out_of_range &e) {
        return Option<char>::none();
    }
}

Option<char> Lexer::advance() {
    // if EOF is hitted return None.
    if (this->cursor >= this->src.length()) return Option<char>::none();

    // updating line counter.
    if (this->peek().is_some_and(
            [](char x) { return x == '\n'; })) this->line++;

    // technically this will never fail.
    return Option<char>::some(this->src.at(this->cursor++));
}

Option<Token> Lexer::next() {
    while (this->peek().is_some()) {
        auto c = this->advance().unwrap();

        switch (c) {
            case '#': {
                while (this->peek().is_some_and(
                    [](char x) { return (x != ' ' && x != '\n'); }
                )) {
                    this->advance();
                }

                auto tkn = this->token();
                if (tkn.text == "#code") tkn.type = TokenType::CODE;
                else if (tkn.text == "#data") tkn.type = TokenType::DATA;
                else {
                    auto msg = token_loc(tkn) + " - unknown section '" + tkn.text + "'.";
                    crash(msg);
                }

                return Option<Token>::some(tkn);
            } break;

            default:
                return Option<Token>::none();
        }
    }
    return Option<Token>::none();
}

void Lexer::reset() {
    // resetting Lexer state.
    this->filepath.clear();
    this->src.clear();
    this->line = 1;
    this->cursor = 0;
    this->old_cursor = 0;
}
