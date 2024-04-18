#ifndef INSTRUCTIONSET_H
#define INSTRUCTIONSET_H

#include "shared/Basic.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
#include <string>

// Forward declaration for the Visitor class.
class Instr;

enum Bool_Op {
    // &&
    BAND,
    // ||
    BOR,
};

// Design Pattern: Visitor.
class Visitor {
    public:
        // TODO: make these pure virtual methods.
        virtual std::string compile_data(std::vector<std::unique_ptr<Instr>> variables) { return ""; } 
        virtual std::string compile_code(std::vector<std::unique_ptr<Instr>> instructions) { return ""; } 
        virtual std::string compile_label(std::string name) { return ""; }
        virtual std::string compile_exit(std::string value) { return ""; }
        virtual std::string compile_add(std::string dst, std::string src) { return ""; }
        virtual std::string compile_sub(std::string dst, std::string src) { return ""; }
        virtual std::string compile_mov(std::string dst, std::string src) { return ""; }
        virtual std::string compile_break() { return ""; }
        virtual std::string compile_enum(std::vector<std::unique_ptr<Instr>> values) { return ""; }
        virtual std::string compile_while(std::vector<std::unique_ptr<Instr>> conditions, 
                                          std::vector<Bool_Op> bool_ops, 
                                          std::vector<std::unique_ptr<Instr>> body) { return ""; }
        virtual std::string compile_for(std::unique_ptr<Instr> range_left, 
                                        std::unique_ptr<Instr> range_right,
                                        std::unique_ptr<Instr> increment,
                                        std::vector<std::unique_ptr<Instr>> body) { return ""; }
        virtual std::string compile_loop(std::vector<std::unique_ptr<Instr>> body) { return ""; }
        virtual std::string compile_var(std::string name, std::string value, bool is_decl) { return ""; }

    private:
        uint_t if_counter = 0;
        uint_t while_counter = 0;
        uint_t for_counter = 0;
        uint_t loop_counter = 0;
        std::string current_loop = "";
};

// Instruction set.
class Instr {
    public:
        virtual ~Instr() = default;

        // TODO: make this a pure virtual method.
        virtual std::string compile(Visitor &v) { 
            crash("If you see this message it means that there is an error inside the Parser.\n");
            return ""; 
        };
};

class Data : public Instr {
    public:
        explicit Data(std::vector<std::unique_ptr<Instr>> variables) : variables(std::move(variables)) {} 

        std::string compile(Visitor &v) { return v.compile_data(std::move(this->variables)); }

        std::vector<std::unique_ptr<Instr>> variables;
};

class Code : public Instr {
    public:
        explicit Code(std::vector<std::unique_ptr<Instr>> instructions) : instructions(std::move(instructions)) {} 

        std::string compile(Visitor &v) { return v.compile_code(std::move(this->instructions)); }

        std::vector<std::unique_ptr<Instr>> instructions;
};

class Label : public Instr {
    public:
        explicit Label(std::string name) : name(name) {}

        std::string compile(Visitor &v) { return v.compile_label(this->name); }

        std::string name;
};

class Exit : public Instr {
    public:
        explicit Exit(std::unique_ptr<Instr> exit_value) : exit_value(std::move(exit_value)) {}

        std::string compile(Visitor &v) { return v.compile_exit(this->exit_value->compile(v)); }

        std::unique_ptr<Instr> exit_value;
};

class Add : public Instr {
    public:
        explicit Add(std::unique_ptr<Instr> dst, std::unique_ptr<Instr> src) : dst(std::move(dst)), src(std::move(src)) {}

        std::string compile(Visitor &v) { return v.compile_add(this->dst->compile(v), this->src->compile(v)); }

        std::unique_ptr<Instr> dst;
        std::unique_ptr<Instr> src;
};

class Sub : public Instr {
    public:
        explicit Sub(std::unique_ptr<Instr> dst, std::unique_ptr<Instr> src) : dst(std::move(dst)), src(std::move(src)) {}

        std::string compile(Visitor &v) { return v.compile_sub(this->dst->compile(v), this->src->compile(v)); }

        std::unique_ptr<Instr> dst;
        std::unique_ptr<Instr> src;
};

class Mov : public Instr {
    public:
        explicit Mov(std::unique_ptr<Instr> dst, std::unique_ptr<Instr> src) : dst(std::move(dst)), src(std::move(src)) {}

        std::string compile(Visitor &v) { return v.compile_mov(this->dst->compile(v), this->src->compile(v)); }

        std::unique_ptr<Instr> dst;
        std::unique_ptr<Instr> src;
};

class Break : public Instr {
    public:
        explicit Break() {}

        std::string compile(Visitor &v) { return v.compile_break(); }
};

class Enum_Var : public Instr {
    public:
        explicit Enum_Var(std::string name, std::vector<std::unique_ptr<Instr>> values) : name(name), values(std::move(values)) {}

        std::string compile(Visitor &v) { return v.compile_enum(std::move(this->values)); }

        std::string name;
        std::vector<std::unique_ptr<Instr>> values;
};

class While : public Instr {
    public:
        explicit While(std::vector<std::unique_ptr<Instr>> conditions, 
                       std::vector<Bool_Op> bool_ops, 
                       std::vector<std::unique_ptr<Instr>> body) 
            : conditions(std::move(conditions)), 
              bool_ops(bool_ops), 
              body(std::move(body)) {}

        std::string compile(Visitor &v) { return v.compile_while(std::move(conditions), std::move(bool_ops), std::move(body)); }

        std::vector<std::unique_ptr<Instr>> conditions;
        std::vector<Bool_Op> bool_ops;
        std::vector<std::unique_ptr<Instr>> body;
};

class For : public Instr {
    public:
        explicit For(std::unique_ptr<Instr> range_left, 
                     std::unique_ptr<Instr> range_right,
                     std::unique_ptr<Instr> increment,
                     std::vector<std::unique_ptr<Instr>> body) 
            : range_left(std::move(range_left)), 
              range_right(std::move(range_right)), 
              increment(std::move(increment)), 
              body(std::move(body)) {}

        std::string compile(Visitor &v) { return v.compile_for(std::move(range_left), 
                                                               std::move(range_right), 
                                                               std::move(increment), 
                                                               std::move(body)); }

        std::unique_ptr<Instr> range_left;
        std::unique_ptr<Instr> range_right;
        std::unique_ptr<Instr> increment;
        std::vector<std::unique_ptr<Instr>> body;
};

class Loop : public Instr {
    public:
        explicit Loop(std::vector<std::unique_ptr<Instr>> body) : body(std::move(body)) {}

        std::string compile(Visitor &v) { return v.compile_loop(std::move(this->body)); }

        std::vector<std::unique_ptr<Instr>> body;
};

// if visitor is in charge of handling the jumps between labels. 
class If : public Instr {
    public:
        explicit If(std::vector<std::unique_ptr<Instr>> conditions,
                    std::vector<Bool_Op> bool_ops,
                    std::vector<std::unique_ptr<Instr>> if_body, 
                    std::vector<std::unique_ptr<Instr>> else_body) 
            : conditions(std::move(conditions)), 
              bool_ops(bool_ops),
              if_body(std::move(if_body)), 
              else_body(std::move(else_body)) {}

        std::vector<std::unique_ptr<Instr>> conditions;
        std::vector<Bool_Op> bool_ops;
        std::vector<std::unique_ptr<Instr>> if_body;
        std::vector<std::unique_ptr<Instr>> else_body;
};

// Enum representing the condition operations.
enum Cond_Op {
    // ==
    EQU,
    // !=
    NEQU,
    // <
    LTH,
    // <=
    LTE,
    // >
    GT,
    // >=
    GTE,
};

class Cond : public Instr {
    public:
        explicit Cond(Cond_Op op, std::unique_ptr<Instr> lhs, std::unique_ptr<Instr> rhs) : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

        Cond_Op op;
        std::unique_ptr<Instr> lhs;
        std::unique_ptr<Instr> rhs;
};

class Var : public Instr {
    public:
        explicit Var(std::string name, std::string value, bool is_decl) : name(name), value(value), is_decl(is_decl) {}

        std::string compile(Visitor &v) { return v.compile_var(this->name, this->value, this->is_decl); }

        std::string name;
        std::string value;
        bool is_decl = true;
};

class Txt : public Instr {
    public:
        explicit Txt(std::string txt) : value(txt) {}

        std::string compile(Visitor &v) { return this->value; }

        std::string value;
};

#endif // INSTRUCTIONSET_H
