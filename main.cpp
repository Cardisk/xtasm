#include <iostream>

#include "./src/front/Lexer.h"
#include "./src/front/Parser.h"
#include "./src/front/Token.h"

#include "./src/back/dll.h"

#define OK 0
#define ERR 1

std::string shift(int &argc, char** &argv) {
    if (!argc) return "";
    argc--;
    auto str = std::string(*argv);
    argv++;
    return str;
}

void print_tokens(std::vector<Token> &vl) {
    std::cout << "----------------\n";
    std::cout << "DEBUG: print_tokens\n";

    std::cout << "len(tokens) = " << vl.size() << std::endl << std::endl;

    for (auto tkn : vl) {
        std::cout << token_loc(tkn) << std::endl;
        std::cout << "'" << tkn.text << "'" << std::endl;
        std::cout << "type: " << ttype_str(tkn.type) << std::endl << std::endl;
    }

    std::cout << "----------------\n";
}

void print_parser_info(std::vector<std::unique_ptr<Instr>> &vp) {
    std::cout << "----------------\n";
    std::cout << "DEBUG: print_parser_info\n";

    std::cout << "len(instructions) = " << vp.size() << std::endl << std::endl;
    std::cout << "len(data) = " << ((Data*)(vp.at(0).get()))->variables.size() << std::endl;
    std::cout << "len(code) = " << ((Code*)(vp.at(1).get()))->instructions.size() << std::endl;

    std::cout << "----------------\n";
}

void usage() {
    std::cout << "Usage: ./xtasm [options] <file>\n";
    std::cout << "Options:\n";
    std::cout << "\t-dbgl: debug the tokens\n";
    std::cout << "\t-dbgp: debug the parser info\n";
}

int main(int argc, char** argv) {
    // skip the program name.
    shift(argc, argv);

    if (!argc) {
        usage();
        return ERR;
    }

    Lexer l = Lexer();
    Parser p = Parser();

    bool debug_tkns = false;
    bool debug_parser = false;

    std::string arg;
    do {
        arg = shift(argc, argv);
        if (arg == "-all") {
            debug_tkns = true;
            debug_parser = true;
        }
        else if (arg == "-dbgl") debug_tkns = true;
        else if (arg == "-dbgp") debug_parser = true;
    } while(argc > 0 && arg.starts_with("-"));

    auto file = "./example/" + arg;
    auto vl = l.lex_file(file);
    auto vp = p.parse_tkns(vl);

    if (debug_tkns) print_tokens(vl);
    if (debug_parser) print_parser_info(vp);

    std::cout << compile("./build/libtemplate.so", vp);

    return OK;
}

