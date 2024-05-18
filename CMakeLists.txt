cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 23)

project(xtasm)

set(SHARED ./src/shared/Basic.h ./src/shared/Basic.cpp ./src/shared/Logger.h ./src/shared/Option.h)

set(TOKEN ./src/front/Token.h ./src/front/Token.cpp)

set(LEXER ./src/front/Lexer.h ./src/front/Lexer.cpp)

set(PARSER ./src/front/Parser.h ./src/front/Parser.cpp)

set(DLL ./src/back/dll.h ./src/back/dll_unix.cpp)

set(FRONT ${TOKEN} ${LEXER} ${PARSER})

set(BACK ${DLL})

add_executable(xtasm main.cpp ${SHARED} ${FRONT} ${BACK})
