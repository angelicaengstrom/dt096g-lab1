#include <iostream>
#include <string>
#include "parser.h"
#include <chrono>

int main(int argc, char *argv[]) {
    if(argc > 1){
        std::string program = argv[1];
        std::string input;
        std::getline(std::cin, input);

        IT begin = program.begin();
        IT end = program.end();
        parser parse = parser(begin, end);

        generator generate = generator(parse.get_match(), input);
        generate.get_result();
    }
    return 0;
}