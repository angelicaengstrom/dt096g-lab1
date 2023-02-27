//
// Created by angel on 2023-02-02.
//

#include "lexer.h"

lexer::token lexer::get_current_token() const {
    return current_token;
}

void lexer::set_current_token(lexer::token token) {
    lexer::current_token = token;
}

lexer::token lexer::lookup(IT& it, IT& last) {
    if(it == last){ return END; }
    switch(*it){
        case '(': return LPAREN;
        case ')': return RPAREN;
        case '{': return LBRACKET;
        case '}': return RBRACKET;
        case '.': return WILDCARD;
        case '+': return EITHER_OP;
        case '*': return MANY_OP;
        case '\\':
            it++;
            if(*it == 'O'){
                it++;
                return LEVEL_OP; }
            if(*it == 'I'){
                it++;
                return IGNORE_OP; }
            break;
    }
    if((*it >= 'A' && *it <= 'Z') || (*it >= 'a' && *it <= 'z') || std::isspace(*it)){
        return LETTER;
    }
    if(*it >= '0' && *it <= '9'){
        return DIGIT;
    }
    throw std::invalid_argument("Unknown lexeme");
}

void lexer::set_current_token(IT& it, IT& last) {
    set_current_token(lookup(it, last));
}

lexer::token lexer::get_current(IT& it, IT& last) {
    set_current_token(lookup(it, last));
    return get_current_token();
}

lexer::token lexer::get_next(IT& it, IT& last) {
    return lookup(++it, last);;
}