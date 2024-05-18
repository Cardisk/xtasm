#include "../InstructionSet.h"

class Concrete_Visitor_Name : public Visitor {
    public: 
        virtual std::string compile_data(std::vector<std::unique_ptr<Instr>> variables) { 
            return "compile_data"; 
        } 
        
        virtual std::string compile_code(std::vector<std::unique_ptr<Instr>> instructions) { 
            return "compile_code"; 
        }

        virtual std::string compile_label(std::string name) { 
            return "compile_label"; 
        }

        virtual std::string compile_exit(std::string value) { 
            return "compile_exit"; 
        }

        virtual std::string compile_add(std::string dst, std::string src) { 
            return "compile_add"; 
        }

        virtual std::string compile_sub(std::string dst, std::string src) { 
            return "compile_sub"; 
        }

        virtual std::string compile_mul(std::string dst, std::string src) { 
            return "compile_mul"; 
        }

        virtual std::string compile_mov(std::string dst, std::string src) { 
            return "compile_mov"; 
        }

        virtual std::string compile_jmp(std::string target) { 
            return "compile_jmp"; 
        }

        virtual std::string compile_break() { 
            return "compile_break"; 
        }

        virtual std::string compile_enum(std::vector<std::unique_ptr<Instr>> values) { 
            return "compile_enum"; 
        }

        virtual std::string compile_while(std::vector<std::unique_ptr<Instr>> conditions, 
                                          std::vector<Bool_Op> bool_ops, 
                                          std::vector<std::unique_ptr<Instr>> body) { 
            return "compile_while"; 
        }

        virtual std::string compile_for(std::unique_ptr<Instr> range_left, 
                                        std::unique_ptr<Instr> range_right,
                                        std::unique_ptr<Instr> increment,
                                        std::vector<std::unique_ptr<Instr>> body) { 
            return "compile_for"; 
        }

        virtual std::string compile_loop(std::vector<std::unique_ptr<Instr>> body) { 
            return "compile_loop"; 
        }

        virtual std::string compile_var(std::string name, std::string value, bool is_decl) { 
            return "compile_var"; 
        }
};

extern "C" {

const char *compile(std::vector<std::unique_ptr<Instr>> &instructions) {
    std::string *buf = new std::string();
    Concrete_Visitor_Name v;

    for (auto &ptr : instructions) {
        *buf += ptr->compile(v) + "\n";
    }

    return buf->c_str();
}

}
