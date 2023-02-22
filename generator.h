//
// Created by angel on 2023-02-11.
//

#ifndef LAB1_GENERATOR_H
#define LAB1_GENERATOR_H

#include <vector>
#include <string>

using IT = std::string::iterator;

struct base{
    virtual std::string evaluate(IT it, IT last) = 0;
    std::vector<base*> children;
    bool ignore = false;
};

struct match:base{
    std::string evaluate(IT it, IT last) override;
};

struct level:base{
    std::string evaluate(IT it, IT last) override;
    IT digit;
    IT start;
};

struct expressions:base{
    std::string evaluate(IT it, IT last) override;
};

struct subexpression:base{
    std::string evaluate(IT it, IT last) override;
};

struct expression:base{
    std::string evaluate(IT it, IT last) override;
};

struct ignore:base{
    std::string evaluate(IT it, IT last) override;
};

struct either:base{
    std::string evaluate(IT it, IT last) override;
};

struct count:base{
    std::string evaluate(IT it, IT last) override;
    std::string::iterator digit;
};

struct many:base{
    std::string evaluate(IT it, IT last) override;
};

struct operand:base{
    std::string evaluate(IT it, IT last) override;
};

struct string:base{
    std::string evaluate(IT it, IT last) override;
};

struct letter:base{
    std::string evaluate(IT it, IT last) override;
    std::string::iterator letter;
};

struct wildcard:base{
    std::string evaluate(IT it, IT last) override;
};

class generator {
private:
    match *matcher;
    std::string input;
public:
    generator(match *m, std::string  input);
    void get_result();
};

#endif //LAB1_GENERATOR_H
