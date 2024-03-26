#include "Parser.h"
#include "Token.h"

#include <memory>
#include <string>
#include <vector>

std::vector<std::unique_ptr<Instr>> Parser::parse_tkns(std::vector<Token> tkns) {
    // initializing the Parser.
    this->tkns = tkns;

    // parsing.
    std::vector<std::unique_ptr<Instr>> ast; 

    while (true) {
        // getting the next instruction.
        auto instr = this->next();
        // validating the instruction.
        if (!instr) break;

        // now it's a valid instruction, can be safely unwrapped.
        ast.push_back(std::move(instr));
    }

    return ast;
}

Option<Token> Parser::peek(uint_t offset) {
    try {
        // if the cursor position is valid return the character.
        return Option<Token>::some(
            this->tkns.at(this->cursor + offset)
        );
    } catch (std::out_of_range &e) {
        // if the cursor position is invalid return None.
        return Option<Token>::none();
    }
}

Option<Token> Parser::advance() {
    // checking if the end is reached.
    if (this->cursor >= this->tkns.size()) return Option<Token>::none();

    // technically this will never fail.
    return Option<Token>::some(this->tkns[this->cursor++]);
}

std::unique_ptr<Instr> Parser::next() {
    // continue getting tokens until the end.
    while (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        switch (tkn.type) {
            case TokenType::DATA: return this->parse_data();
            case TokenType::CODE: return this->parse_code();
            default: {
                auto msg = "Unexpected token '" + tkn.text + "' (Not a valid instruction)\n";
                msg += "\t\tfound at -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // return nullptr if the parsing is completed.
    return nullptr; 
}

std::unique_ptr<Data> Parser::parse_data() {
    std::vector<std::unique_ptr<Instr>> variables;

    while (this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::VAR; }
    )) {
        auto var = this->parse_variable();
        if (!var) break;

        variables.push_back(std::move(var));
    }

    return std::make_unique<Data>(std::move(variables));
}

std::unique_ptr<Code> Parser::parse_code() {
    std::vector<std::unique_ptr<Instr>> instructions;

    while (this->peek().is_some()) {
        auto tkn = this->advance().unwrap();

        switch (tkn.type) {
            case TokenType::EXIT: {
                auto exit = this->parse_exit();
                if (!exit) break;

                instructions.push_back(std::move(exit));
            } break;

            default: {
                std::string msg = "Unexpected token '" + tkn.text + "' (Not a valid instruction)\n";
                msg += "\t\tfound at -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    return std::make_unique<Code>(std::move(instructions));
}

std::unique_ptr<Exit> Parser::parse_exit() {
    // checking for a value.
    if (this->peek().is_none()) {
        std::string msg = "Missing value for EXIT instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    // checking for a valid value.
    if (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::INT; }
    )) {
        // now it's a safe unwrapping, no need to consume.
        auto tkn = this->peek().unwrap();
        std::string msg = "Invalid parameter used for EXIT instruction\n";
        msg += "\tfound -- '" + tkn.text + "'\n";
        msg += "\tat    -- " + token_loc(tkn);
        // crashing the compiler.
        crash(msg);
    }

    // now the token has been validated.
    auto tkn = this->advance().unwrap();
    return std::make_unique<Exit>(std::stoi(tkn.text));
}

std::unique_ptr<Var> Parser::parse_variable() {
    // no variables declared.
    if (this->peek().is_none()) return nullptr;

    // checking for a valid variable.
    if (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::VAR; }
    )) {
        // now it's a safe unwrapping, no need to consume.
        auto tkn = this->peek().unwrap();
        std::string msg = "Invalid variable declaration\n";
        msg += "\tfound -- '" + tkn.text + "'\n";
        msg += "\tat    -- " + token_loc(tkn);
        // crashing the compiler.
        crash(msg);
    }

    // this is a safe, variable name already checked.
    // removing the '.' from the variable name.
    std::string name = this->advance().unwrap().text.erase(0, 1);
    // maybe there isn't a value, so initializing it with an empty string.
    std::string value = "";

    if (this->peek().is_none()) {
        std::string msg = "Missing value for variable '" + name + "'\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    auto tkn = this->peek().unwrap();
    switch (tkn.type) {
        case TokenType::INT:
            value = tkn.text;
            break;

        case TokenType::QMARK:
            // empty variable.
            break;

        default: {
            std::string msg = "Invalid value for variable '" + name + "'\n";
            msg += "\tfound -- '" + this->peek().unwrap().text + "'\n";
            msg += "\tat    -- " + token_loc(tkn);
            // crashing the compiler.
            crash(msg);
        } break;
    }

    value = this->advance().unwrap().text;
    
    return std::make_unique<Var>(name, value);
}

