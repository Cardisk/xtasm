#include <iostream>

#include "./src/front/Lexer.h"

int main(void) {
    Lexer l = Lexer();
    // TODO: testing
    auto file = "./example/test.xt";
    auto v = l.lex_file(file);

    std::cout << "len(tokens) = " << v.size() << std::endl; 

    for (auto tkn : v) {
        std::cout << tkn.text << std::endl;
    }
    return 0;
}
