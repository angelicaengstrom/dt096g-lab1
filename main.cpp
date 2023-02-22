#include <iostream>
#include <string>
#include "parser.h"

int main(int argc, char *argv[]) {
    if(argc > 1){
        static std::string program = argv[1];
        std::string input;
        std::getline(std::cin, input);

        parser parse = parser(program.begin(), program.end());

        runner run = runner(parse.get_match(), input);
        run.get_result();
    }
    return 0;
}