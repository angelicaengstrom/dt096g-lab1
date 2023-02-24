//
// Created by angel on 2023-02-11.
//

#ifndef LAB1_GENERATOR_H
#define LAB1_GENERATOR_H

#include <vector>
#include <string>

using IT = std::string::iterator;

struct base{
    virtual bool evaluate(IT &it, IT &last) = 0;
    std::vector<base*> children;
    bool ignore = false;
};

struct match:base{
    bool evaluate(IT &it, IT &last) override;
};

struct level:base{
    bool evaluate(IT &it, IT &last) override;
    IT digit;
};

struct expressions:base{
    bool evaluate(IT &it, IT &last) override;
};

struct subexpression:base{
    bool evaluate(IT &it, IT &last) override;
};

struct expression:base{
    bool evaluate(IT &it, IT &last) override;
};

struct ignore:base{
    bool evaluate(IT &it, IT &last) override;
};

struct either:base{
    bool evaluate(IT &it, IT &last) override;
};

struct count:base{
    bool evaluate(IT &it, IT &last) override;
    IT digit;
};

struct many:base{
    bool evaluate(IT &it, IT &last) override;
};

struct operand:base{
    bool evaluate(IT &it, IT &last) override;
};

struct string:base{
    bool evaluate(IT &it, IT &last) override;
};

struct letter:base{
    bool evaluate(IT &it, IT &last) override;
    IT letter;
};

struct wildcard:base{
    bool evaluate(IT &it, IT &last) override;
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