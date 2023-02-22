//
// Created by angel on 2023-02-02.
//

#ifndef LAB1_LEXER_H
#define LAB1_LEXER_H
#include <iostream>

class lexer {
public:
    enum token{
        LPAREN, RPAREN, LBRACKET, RBRACKET,
        EITHER_OP, MANY_OP,
        WILDCARD, DIGIT, LETTER,
        LEVEL_OP, IGNORE_OP,
        UNKNOWN, END
    };

    [[nodiscard]] token get_current_token() const;

    void set_current_token(token token);

    void set_current_token(std::string::iterator it, std::string::iterator last);

    token get_current(std::string::iterator it, std::string::iterator last);

    token get_next(std::string::iterator it, std::string::iterator last);
private:
    token lookup(std::string::iterator it, std::string::iterator last);
    token current_token;
};


#endif //LAB1_LEXER_H
