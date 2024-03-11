#include "Lexer.h"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <cerrno>
#include <string>

#include "Token.h"

std::vector<Token> Lexer::lex_file(std::string filepath) {
    // reset the Lexer state.
    this->reset();
    this->filepath = filepath;
    
    // read the content of the file.
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
    
    // tokenization.
    std::vector<Token> tkns;

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
        .column = this->column,
    };

    // shifting forward the old_cursor to point after the token.
    this->old_cursor = this->cursor;

    return tkn;
}

Option<char> Lexer::peek(uint_t offset) {
    try {
        // if the cursor position is valid return the character.
        return Option<char>::some(
            this->src.at(this->cursor + offset)
        );
    } catch (std::out_of_range &e) {
        // if the cursor position is invalid return None.
        return Option<char>::none();
    }
}

Option<char> Lexer::advance() {
    // if EOF is hitted return None.
    if (this->cursor >= this->src.length()) return Option<char>::none();

    // technically this will never fail.
    return Option<char>::some(this->src.at(this->cursor++));
}

void Lexer::new_line() {
    // update line and column.
    this->line++;
    this->column = 1;
}

Option<Token> Lexer::next() {
    // continue getting chars until a token or EOF is found.
    while (this->peek().is_some()) {
        // now this is safe.
        auto c = this->advance().unwrap();

        switch (c) {
            case '#': {
                // consume the token.
                while (this->peek().is_some_and(
                    [](char x) { return (x != ' ' && x != '\n'); }
                )) {
                    this->advance();
                }

                auto tkn = this->token();

                // token type recognition.
                if (tkn.text == "#code") tkn.type = TokenType::CODE;
                else if (tkn.text == "#data") tkn.type = TokenType::DATA;
                else {
                    // if here something wrong is inside the file.
                    auto msg = token_loc(tkn) + " - unknown section '" + tkn.text + "'.";
                    crash(msg);
                }

                return Option<Token>::some(tkn);
            } break;

            // ignoring spaces.
            case ' ':
                this->old_cursor++;
                this->column++;
                break;

            // ignoring new lines.
            case '\n':
                this->old_cursor++;
                this->new_line();
                break;

            default: {
                // checking for keywords.
                if (std::isalpha(c)) {
                    // consuming the token.
                    while (this->peek().is_some_and(
                        [](char x) { return (bool) std::isalpha(x); }
                    )) {
                        this->advance();
                    }

                    auto tkn = this->token();

                    // checking word existence
                    if (tkn.text == "exit") tkn.type = TokenType::EXIT;
                    // TODO: put here all the known instructions.
                    // XXX: handle also some user-defined in a final branch.
                    
                    return Option<Token>::some(tkn);
                }
            } break;
        }
    }
    return Option<Token>::none();
}

void Lexer::reset() {
    // resetting Lexer state.
    this->filepath.clear();
    this->src.clear();
    this->line = 1;
    this->column = 1;
    this->cursor = 0;
    this->old_cursor = 0;
}
