#include "Parser.h"
#include "Token.h"

#include <iostream>
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
    // data section is empty.
    std::vector<std::unique_ptr<Instr>> variables;

    // parsing the variables.
    while (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::CODE; }
    )) {
        auto tkn = this->peek().unwrap();

        switch (tkn.type) {
            case TokenType::VAR: {
                auto var = this->parse_variable();
                if (!var) break;

                variables.push_back(std::move(var));
            } break;

            case TokenType::ENUM: {
                // skipping the ENUM keyword. 
                this->advance();
                auto enum_var = this->parse_enum();
                if (!enum_var) break;

                variables.push_back(std::move(enum_var));
            } break;

            default: 
                break;
        }
    }
    std::cout << "#data parsed\n";

    // returning the parsed data.
    return std::make_unique<Data>(std::move(variables));
}

std::unique_ptr<Code> Parser::parse_code() {
    // code section is empty.
    std::vector<std::unique_ptr<Instr>> instructions;

    // parsing the instructions.
    while (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::END; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible instructions.
        switch (tkn.type) {
            case TokenType::EXIT: {
                auto exit = this->parse_exit();
                if (!exit) break;

                instructions.push_back(std::move(exit));
            } break;

            case TokenType::ADD: {
                auto add = this->parse_add();
                if (!add) break;

                instructions.push_back(std::move(add));
            } break;

            case TokenType::SUB: {
                auto sub = this->parse_sub();
                if (!sub) break;

                instructions.push_back(std::move(sub));
            } break;

            case TokenType::MOV: {
                auto mov = this->parse_mov();
                if (!mov) break;

                instructions.push_back(std::move(mov));
            } break;

            default: {
                std::string msg = "Unexpected token '" + tkn.text + "' (Not a valid instruction)\n";
                msg += "\t\tfound at -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // returning the parsed code.
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

    std::unique_ptr<Instr> value;

    // checking for a valid value.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                value = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::REG:
            case TokenType::INT: {
                value = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid parameter used for EXIT instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // now the value has been parsed.
    return std::make_unique<Exit>(std::move(value));
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
    
    return std::make_unique<Var>(name, value, true);
}

std::unique_ptr<Add> Parser::parse_add() {
    // checking for a value.
    if (this->peek().is_none()) {
        std::string msg = "Missing values for ADD instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    std::unique_ptr<Instr> lhs;
    std::unique_ptr<Instr> rhs;

    // checking for a valid lhs.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                lhs = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::REG: {
                lhs = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid left hand side for ADD instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }
    
    // checking for a valid rhs.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                rhs = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::INT:
            case TokenType::REG: {
                rhs = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid right hand side for ADD instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // now the values have been parsed.
    return std::make_unique<Add>(std::move(lhs), std::move(rhs));
}

std::unique_ptr<Sub> Parser::parse_sub() {
    // checking for a value.
    if (this->peek().is_none()) {
        std::string msg = "Missing values for SUB instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    std::unique_ptr<Instr> lhs;
    std::unique_ptr<Instr> rhs;

    // checking for a valid lhs.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                lhs = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::REG: {
                lhs = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid left hand side for SUB instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }
    
    // checking for a valid rhs.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                rhs = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::INT:
            case TokenType::REG: {
                rhs = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid right hand side for SUB instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // now the values have been parsed.
    return std::make_unique<Sub>(std::move(lhs), std::move(rhs));
}

std::unique_ptr<Mov> Parser::parse_mov() {
    // checking for a value.
    if (this->peek().is_none()) {
        std::string msg = "Missing values for MOV instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    std::unique_ptr<Instr> dst;
    std::unique_ptr<Instr> src;

    // checking for a valid dst.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                dst = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::REG: {
                dst = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid destination for MOV instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }
    
    // checking for a valid src.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                src = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::INT:
            case TokenType::REG: {
                src = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid source for MOV instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // now the values have been parsed.
    return std::make_unique<Mov>(std::move(dst), std::move(src));
}

std::unique_ptr<Enum_Var> Parser::parse_enum() {
    // enum error.
    if (this->peek().is_none()) {
        std::string msg = "Missing values for ENUM instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    auto name = this->advance().unwrap().text;

    std::vector<std::unique_ptr<Instr>> values;
    int enum_index = 0;
    while (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::END; }
    )) {
        auto tkn_name = this->advance().unwrap();

        if (tkn_name.type != TokenType::VAR) {
            std::string msg = "Invalid value for ENUM declaration.\n\tfound -- '" + tkn_name.text + "'\n";
            msg += "\tat -- " + token_loc(tkn_name);
            // crashing the compiler.
            crash(msg);
        }

        if (this->peek().is_some_and( 
                [](Token x) { return x.type == TokenType::INT; } 
        )) {
            enum_index = std::stoi(this->advance().unwrap().text);
        }

        values.push_back(std::make_unique<Var>(
            tkn_name.text, std::to_string(enum_index), true
        ));
        
        enum_index++;
    }

    // consuming the END token.
    if (this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::END; }
    )) {
        this->advance();
    }

    return std::make_unique<Enum_Var>(name, std::move(values));
}


