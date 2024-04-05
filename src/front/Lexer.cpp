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
    
    if (filepath.empty()) crash("No file provided.");
    if (!filepath.ends_with(".xt")) crash("Invalid file extension. Expected '.xt'");

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

            case '$': {
                // consume the token.
                while (this->peek().is_some_and(
                    [](char x) { return (x != ' ' && x != '\n'); }
                )) {
                    this->advance();
                }

                auto tkn = this->token();
                tkn.type = TokenType::REG;
                tkn.text.replace(0, 1, "");

                return Option<Token>::some(tkn);
            } break;

            // label definition.
            case ':': {
                // consume the token.
                while (this->peek().is_some_and(
                    [](char x) { return (x != ' ' && x != '\n'); }
                )) {
                    this->advance();
                }

                auto tkn = this->token();
                tkn.type = TokenType::LABEL;
                tkn.text.replace(0, 1, "");

                return Option<Token>::some(tkn);
            } break;

            // boolean eq.
            case '=': {
               if (!this->peek().is_some_and(
                   [](char x) { return x == '='; }
               )) {
                   auto tkn = this->token();
                   std::string msg = "Unexpected character '=' (Unfinished boolean eq)\n";
                   msg += "\tfound at -- " + token_loc(tkn);
                   crash(msg);
               }
               this->advance();

               auto tkn = this->token();
               tkn.type = TokenType::EQ;
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
            
            // ignoring comments.
            case '-': {
                if (!this->peek().is_some_and(
                    [](char x) { return x == '-'; }
                )) {
                    auto tkn = this->token();
                    auto msg = "Unexpected character '-' (Unfinished comment prefix)\n\tfound at -- " + token_loc(tkn);
                    crash(msg);
                }

                // consume the line untile '\n'.
                while (this->peek().is_some_and(
                    [](char x) { return x != '\n'; }
                )) {
                    this->old_cursor++;
                    this->advance();
                }

                this->old_cursor++;
            } break;

            // variable initialization inside bss.
            case '?': {
                auto tkn = this->token();
                tkn.type = TokenType::QMARK;
                return Option<Token>::some(tkn);
            }

            default: {
                // checking for keywords.
                if (std::isalpha(c) || c == '.') {
                    // consuming the token.
                    while (this->peek().is_some_and(
                        [](char x) { return (bool) std::isalpha(x) || x == '_' || x == '.'; }
                    )) {
                        this->advance();
                    }

                    auto tkn = this->token();

                    // checking word existence
                    
                    if      (tkn.text.starts_with('.')) tkn.type = TokenType::VAR;
                    // TODO: put here all the known instructions.
                    else if (tkn.text == "exit") tkn.type = TokenType::EXIT;
                    else if (tkn.text == "add")  tkn.type = TokenType::ADD;
                    else if (tkn.text == "sub")  tkn.type = TokenType::SUB;
                    else if (tkn.text == "mov")  tkn.type = TokenType::MOV;
                    else if (tkn.text == "enum") tkn.type = TokenType::ENUM;
                    else if (tkn.text == "if")   tkn.type = TokenType::IF;
                    else if (tkn.text == "in")   tkn.type = TokenType::IN;
                    else if (tkn.text == "else") tkn.type = TokenType::ELSE;
                    else if (tkn.text == "end")  tkn.type = TokenType::END;
                    else tkn.type = TokenType::NAME;
                    
                    return Option<Token>::some(tkn);
                } 
                
                if (std::isdigit(c)) {
                    // consuming the token.
                    while (this->peek().is_some_and(
                        [](char x) { return ((bool) std::isdigit(x) || x == '.'); }
                    )) {
                        this->advance();
                    }

                    auto tkn = this->token();

                    // checking for the correct number type.
                    // if (tkn.text.contains('.')) tkn.type = TokenType::DEC;
                    // else tkn.type = TokenType::INT;

                    // TODO: at the moment nothing except INT will be handled.
                    // XXX: to reintroduce the tokenization of floats uncomment the upper
                    //      section.
                    tkn.type = TokenType::INT;

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
