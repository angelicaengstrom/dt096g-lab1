//
// Created by angel on 2023-02-02.
//

#ifndef LAB1_PARSER_H
#define LAB1_PARSER_H
#include <iostream>
#include <vector>
#include "lexer.h"
#include "generator.h"

class parser {
    match* parse_match();
    level* parse_level();
    expressions* parse_expressions();
    expression* parse_expression();
    subexpression* parse_subexpression();
    greedy* parse_greedy();
    either* parse_either(operand* op);
    many* parse_many(operand* op);
    count* parse_count(operand* op);
    operand* parse_operand();
    ignore* parse_ignore(string* s);
    ignore* parse_ignore(subexpression* sub);
    string* parse_string();
    match* matcher{};
    lexer lex{};
    IT it;
    IT last;
public:
    parser(IT first, IT last);
    [[nodiscard]] match *get_match() const;
    void set_match(match *matcher);
};


#endif //LAB1_PARSER_H