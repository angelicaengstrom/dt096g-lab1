//
// Created by angel on 2023-02-02.
//

#ifndef LAB1_PARSER_H
#define LAB1_PARSER_H
#include <iostream>
#include <vector>
#include "lexer.h"
#include "runner.h"

class parser {
    match* parse_match();
    level* parse_level();
    ignore* parse_ignore();
    expressions* parse_expressions();
    expression* parse_expression();
    subexpression* parse_subexpression();
    either* parse_either();
    many* parse_many();
    count* parse_count();
    string* parse_string();
    match* matcher;
    lexer lex;
    std::string::iterator it;
    std::string::iterator last;
public:
    parser(std::string::iterator first, std::string::iterator last);
    [[nodiscard]] match *get_match() const;
    void set_match(match *matcher);
};


#endif //LAB1_PARSER_H
