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

    // returning the parsed data.
    return std::make_unique<Data>(std::move(variables));
}

std::unique_ptr<Instr> Parser::parse(Token tkn) {
    // switching all the possible instructions.
    switch (tkn.type) {
        case TokenType::LABEL: 
            return std::make_unique<Label>(tkn.text);

        case TokenType::WHILE: 
            return this->parse_while();

        case TokenType::FOR:
            return this->parse_for();

        case TokenType::LOOP:
            return this->parse_loop();

        case TokenType::IF: 
            return this->parse_if();

        case TokenType::EXIT: 
            return this->parse_exit();

        case TokenType::ADD: 
            return this->parse_add();

        case TokenType::SUB: 
            return this->parse_sub();

        case TokenType::MOV: 
            return this->parse_mov();

        case TokenType::JMP:
            return this->parse_jmp();

        case TokenType::BREAK:
            return this->parse_break();

        default: {
            std::string msg = "Unexpected token '" + tkn.text + "' (Not a valid instruction)\n";
            msg += "\t\tfound at -- " + token_loc(tkn);
            // crashing the compiler.
            crash(msg);
        } break;
    }

    return nullptr;
}

std::unique_ptr<Code> Parser::parse_code() {
    // code section is empty.
    std::vector<std::unique_ptr<Instr>> instructions;

    // parsing the instructions.
    while (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::DATA; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        auto instr = this->parse(tkn);
        if (!instr) break;
        instructions.push_back(std::move(instr));
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

std::unique_ptr<Jmp> Parser::parse_jmp() {

    if (this->peek().is_none()) {
        std::string msg = "Missing target for JMP instruction\n";
        msg += "\tfound at -- " + token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::LABEL; }
    )) {
        auto tkn = this->advance().unwrap();
        std::string msg = "Invalid target for JMP instruction (Not a label)\n";
        msg += "\tfound -- '" + tkn.text + "'\n";
        msg += "\tat    -- " + token_loc(tkn);
        // crashing the compiler.
        crash(msg);
    }

    // now it's safe.
    auto tkn = this->advance().unwrap();
    auto target = std::make_unique<Txt>(tkn.text);

    return std::make_unique<Jmp>(std::move(target));
}

std::unique_ptr<Break> Parser::parse_break() {
    return std::make_unique<Break>();
}

std::unique_ptr<Enum_Var> Parser::parse_enum() {
    // enum error.
    if (this->peek().is_none()) {
        std::string msg = "Missing name for ENUM instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    auto name = this->advance().unwrap().text;

    std::vector<std::unique_ptr<Instr>> values;
    int enum_index = 0;
    while (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::END; }
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

        // crafting a name like '.enum_name.var_name'.
        auto var_name = name + tkn_name.text;

        values.push_back(std::make_unique<Var>(
            var_name, std::to_string(enum_index), true
        ));
        
        enum_index++;
    }

    // consuming the END token.
    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::END; }
    )) {
        std::string msg = "Missing END token for ENUM declaration\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    } 
   
    this->advance();

    return std::make_unique<Enum_Var>(name, std::move(values));
}

std::unique_ptr<If> Parser::parse_if() {
    // checking for a condition.
    // if the condition is missing, parse_cond() will handle it.
    std::vector<std::unique_ptr<Instr>> conditions;
    std::vector<Bool_Op> bool_ops;

    while (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::IN; }
    )) {
        auto cond = this->parse_cond();

        conditions.push_back(std::move(cond));

        if (this->peek().is_none()) {
            std::string msg = "Missing 'IN' keyword for IF instruction\n\tfound at -- ";
            msg += token_loc(this->tkns[this->cursor - 1]);
            // crashing the compiler.
            crash(msg);
        }

        // checking for a boolean operator.
        if (this->peek().is_some_and(
            [](Token x) { return x.type == TokenType::AND || x.type == TokenType::OR; }
        )) {
            auto tkn = this->advance().unwrap();

            Bool_Op op;
            switch (tkn.type) {
                case TokenType::AND:
                    op = Bool_Op::BAND;
                    break;

                case TokenType::OR:
                    op = Bool_Op::BOR;
                    break;

                default: {
                    std::string msg = "Invalid boolean operator (Only && , || are valid)\n\tfound at -- ";
                    msg += token_loc(tkn);
                    // crashing the compiler.
                    crash(msg);
                } break;
            }

            bool_ops.push_back(op);
        }
    }

    // checking for IN keyword.
    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::IN; }
    )) {
        std::string msg = "Missing 'IN' keyword for IF instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    // consuming the IN keyword.
    this->advance();

    // consuming the if body.
    std::vector<std::unique_ptr<Instr>> if_body;

    while (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::ELSE && x.type != TokenType::END; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        auto instr = this->parse(tkn);
        if (!instr) break;

        if_body.push_back(std::move(instr));
    }

    if (this->peek().is_none()) {
        std::string msg = "Missing closing token for IF instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    // now it's safe.
    auto tkn = this->advance().unwrap();

    // consuming the else body.

    // if END is encountered, there is no else body.
    std::vector<std::unique_ptr<Instr>> else_body;

    if (tkn.type == TokenType::END) {
        return std::make_unique<If>(std::move(conditions), bool_ops, std::move(if_body), std::move(else_body));
    }

    // checking for an else if statement.
    if (this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::IF; }
    )) {
        this->advance();

        auto instr = this->parse_if();

        else_body.push_back(std::move(instr));
        return std::make_unique<If>(std::move(conditions), bool_ops, std::move(if_body), std::move(else_body));
    }

    // otherwise, there is an else body.
    while (this->peek().is_some_and(
        [](Token x) { return x.type != TokenType::END; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        auto instr = this->parse(tkn);
        if (!instr) break;

        else_body.push_back(std::move(instr));
    }

    // consuming the END token.
    this->advance();

    return std::make_unique<If>(std::move(conditions), bool_ops, std::move(if_body), std::move(else_body));
}

std::unique_ptr<While> Parser::parse_while() {

    std::vector<std::unique_ptr<Instr>> conditions;
    std::vector<Bool_Op> bool_ops;

    while (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::IN; }
    )) {
        auto cond = this->parse_cond();

        conditions.push_back(std::move(cond));

        if (this->peek().is_none()) {
            std::string msg = "Missing 'IN' keyword for IF instruction\n\tfound at -- ";
            msg += token_loc(this->tkns[this->cursor - 1]);
            // crashing the compiler.
            crash(msg);
        }

        // checking for a boolean operator.
        if (this->peek().is_some_and(
            [](Token x) { return x.type == TokenType::AND || x.type == TokenType::OR; }
        )) {
            auto tkn = this->advance().unwrap();

            Bool_Op op;
            switch (tkn.type) {
                case TokenType::AND:
                    op = Bool_Op::BAND;
                    break;

                case TokenType::OR:
                    op = Bool_Op::BOR;
                    break;

                default: {
                    std::string msg = "Invalid boolean operator (Only && , || are valid)\n\tfound at -- ";
                    msg += token_loc(tkn);
                    // crashing the compiler.
                    crash(msg);
                } break;
            }

            bool_ops.push_back(op);
        }
    }

    // checking for IN keyword.
    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::IN; }
    )) {
        std::string msg = "Missing 'IN' keyword for WHILE instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    // consuming the IN keyword.
    this->advance();

    // consuming the while body.
    std::vector<std::unique_ptr<Instr>> body;

    while (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::END; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        auto instr = this->parse(tkn);
        if (!instr) break;

        body.push_back(std::move(instr));
    }

    if (this->peek().is_none()) {
        std::string msg = "Missing closing token for WHILE instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }
    
    // consuming the END token.
    this->advance();

    return std::make_unique<While>(std::move(conditions), bool_ops, std::move(body));
}

std::unique_ptr<For> Parser::parse_for() {

    std::unique_ptr<Instr> range_left;
    std::unique_ptr<Instr> range_right;
    std::unique_ptr<Instr> increment;
    std::vector<std::unique_ptr<Instr>> body;

    // checking for a valid range_left.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                range_left = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::INT:
            case TokenType::REG: {
                range_left = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid range lower bound for FOR instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // checking for range separator.
    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::SEMICOLON; }
    )) {
        std::string msg = "Missing ';' separator inside FOR instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }
    
    // consuming the separator.
    this->advance();

    // checking for a valid range_right.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                range_right = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::INT:
            case TokenType::REG: {
                range_right = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid range upper bound for FOR instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // checking for range separator.
    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::SEMICOLON; }
    )) {
        std::string msg = "Missing ';' separator inside FOR instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    // consuming the separator.
    this->advance();

    // checking for a valid range increment.
    if (this->peek().is_some()) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        // switching all the possible values.
        switch (tkn.type) {
            case TokenType::VAR: {
                increment = std::make_unique<Var>(tkn.text, "", false);
            } break;

            case TokenType::INT:
            case TokenType::REG: {
                increment = std::make_unique<Txt>(tkn.text);
            } break;

            default: {
                std::string msg = "Invalid range increment for FOR instruction\n";
                msg += "\tfound -- '" + tkn.text + "'\n";
                msg += "\tat    -- " + token_loc(tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        }
    }

    // checking for IN keyword.
    if (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::IN; }
    )) {
        std::string msg = "Missing 'IN' keyword for FOR instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }
    
    // consuming the IN keyword.
    this->advance();

    while (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::END; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        auto instr = this->parse(tkn);
        if (!instr) break;

        body.push_back(std::move(instr));
    }

    if (this->peek().is_none()) {
        std::string msg = "Missing closing token for FOR instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }
    
    // consuming the END token.
    this->advance();
    
    return std::make_unique<For>(std::move(range_left), std::move(range_right), std::move(increment), std::move(body));
}

std::unique_ptr<Loop> Parser::parse_loop() {

    std::vector<std::unique_ptr<Instr>> body;

    while (!this->peek().is_some_and(
        [](Token x) { return x.type == TokenType::END; }
    )) {
        // now it's safe.
        auto tkn = this->advance().unwrap();

        auto instr = this->parse(tkn);
        if (!instr) break;

        body.push_back(std::move(instr));
    }

    if (this->peek().is_none()) {
        std::string msg = "Missing closing token for FOR instruction\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }
    
    // consuming the END token.
    this->advance();

    return std::make_unique<Loop>(std::move(body));
}

std::unique_ptr<Cond> Parser::parse_cond() {

    // validating the lhs.
    if (this->peek().is_none()) {
        std::string msg = "Incomplete condition\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    auto lhs_tkn = this->advance().unwrap();
    std::unique_ptr<Instr> lhs;

    switch (lhs_tkn.type) {
        case TokenType::VAR: 
            lhs = std::make_unique<Var>(lhs_tkn.text, "", false);
            break;

        case TokenType::REG:
        // TODO: add here other possible values.
        case TokenType::INT:
            lhs = std::make_unique<Txt>(lhs_tkn.text);
            break;

        default: {
            std::string msg = "Invalid left hand side for condition (Expected variable, register or value)\n";
            msg += "\tfound -- '" + lhs_tkn.text + "'\n";
            msg += "\tat    -- " + token_loc(lhs_tkn);
            // crashing the compiler.
            crash(msg);
        } break;
    }

    if (this->peek().is_none()) {
        std::string msg = "Missing operator for the condition\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    // validating the operator.
    auto op_tkn = this->advance().unwrap();
    Cond_Op op;

    switch (op_tkn.type) {
        case TokenType::EQ:
            op = Cond_Op::EQU;
            break;

        case TokenType::NEQ:
            op = Cond_Op::NEQU;
            break;

        case TokenType::GRT:
            op = Cond_Op::GT;
            break;

        case TokenType::GEQ:
            op = Cond_Op::GTE;
            break;

        case TokenType::LT:
            op = Cond_Op::LTH;
            break;

        case TokenType::LEQ:
            op = Cond_Op::LTE;
            break;
    
        default: {
            std::string msg = "Invalid boolean operator (Only == , != , > , >= are valid)\n\tfound at -- ";
            msg += token_loc(op_tkn);
            // crashing the compiler.
            crash(msg);
        } break;
    }

    // validating the rhs.
    if (this->peek().is_none()) {
        std::string msg = "Missing right hand side of the condition\n\tfound at -- ";
        msg += token_loc(this->tkns[this->cursor - 1]);
        // crashing the compiler.
        crash(msg);
    }

    auto rhs_tkn = this->advance().unwrap();
    std::unique_ptr<Instr> rhs;

    switch (lhs_tkn.type) {
        case TokenType::VAR: 
            lhs = std::make_unique<Var>(rhs_tkn.text, "", false);
            break;

        case TokenType::REG:
        // TODO: add here other possible values.
        case TokenType::INT:
            lhs = std::make_unique<Txt>(rhs_tkn.text);
            break;

        default: {
            std::string msg = "Invalid right hand side for condition (Expected variable, register or value)\n";
            msg += "\tfound -- '" + lhs_tkn.text + "'\n";
            msg += "\tat    -- " + token_loc(rhs_tkn);
            // crashing the compiler.
            crash(msg);
        } break;
    }

    if (lhs_tkn.type == rhs_tkn.type) {
        switch (lhs_tkn.type) {
            // TODO: add here other possible values.
            case TokenType::INT: {
                std::string msg = "Invalid condition (Both sides can't be values)\n\tfound at -- ";
                msg += token_loc(rhs_tkn);
                // crashing the compiler.
                crash(msg);
            } break;
        
            default: 
                break;
        }
    }
 
    return std::make_unique<Cond>(op, std::move(lhs), std::move(rhs));
}

