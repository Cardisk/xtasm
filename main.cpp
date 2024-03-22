#include <iostream>

#include "./src/front/Lexer.h"
#include "./src/front/Parser.h"
#include "src/front/Token.h"

int main(void) {
    Lexer l = Lexer();
    // TODO: testing
    auto file = "./example/test.xt";
    auto vl = l.lex_file(file);

    std::cout << "len(tokens) = " << vl.size() << std::endl << std::endl; 

    for (auto tkn : vl) {
        std::cout << token_loc(tkn) << std::endl;
        std::cout << "'" << tkn.text << "'" << std::endl;
        std::cout << "type: " << ttype_str(tkn.type) << std::endl << std::endl;
    }

    std::cout << "----------------\n";

    Parser p = Parser();
    auto vp = p.parse_tkns(vl); 

    std::cout << "len(instructions) = " << vp.size() << std::endl << std::endl;
    
    return 0;
}
