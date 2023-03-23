//
// Created by angel on 2023-02-02.
//

#ifndef LAB1_LEXER_H
#define LAB1_LEXER_H
#include <iostream>

using IT = std::string::iterator;

class lexer {
public:
    enum token{
        LPAREN, RPAREN, LBRACKET, RBRACKET,
        EITHER_OP, MANY_OP,
        WILDCARD, DIGIT, LETTER,
        LEVEL_OP, IGNORE_OP,
        UNKNOWN [[maybe_unused]], END
    };

    [[nodiscard]] token get_current_token() const;

    void set_current_token(token token);

    [[maybe_unused]] void set_current_token(IT& it, IT& last);

    token get_current(IT& it, IT& last);

    [[maybe_unused]] token get_next(IT& it, IT& last);
private:
    token lookup(IT it, IT last);
    token current_token;
};


#endif //LAB1_LEXER_H