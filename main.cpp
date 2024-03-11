#include <iostream>

#include "./src/front/Lexer.h"
#include "src/front/Token.h"

int main(void) {
    Lexer l = Lexer();
    // TODO: testing
    auto file = "./example/test.xt";
    auto v = l.lex_file(file);

    std::cout << "len(tokens) = " << v.size() << std::endl << std::endl; 

    for (auto tkn : v) {
        std::cout << token_loc(tkn) << std::endl;
        std::cout << "'" << tkn.text << "'" << std::endl;
        std::cout << "type: " << ttype_str(tkn.type) << std::endl << std::endl;
    }
    return 0;
}
