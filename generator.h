//
// Created by angel on 2023-02-11.
//

#ifndef LAB1_GENERATOR_H
#define LAB1_GENERATOR_H

#include <utility>
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

/*struct greedy:base{
    bool evaluate(IT &it, IT &last) override;
};*/

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

class text{
    std::string escape_code = "\x1b[44m";
    std::string txt;
    std::string end = "\033[0m";
public:
    void set_text(IT t){
        txt = *t;
    }

    [[nodiscard]] std::string get_text() const{
        return escape_code + txt + end;
    }

    std::string get_text(){
        return escape_code + txt + end;
    }

    void update(){
        if(escape_code == "\x1b[44m"){ escape_code = "\x1b[45m"; }else{ escape_code = "\x1b[44m"; }
    }
};

class generator {
    match *matcher;
    std::string input;
    text t;
public:
    generator(match *m, std::string  input);
    void get_result();
    friend std::ostream& operator<<(std::ostream& out, const text& t);
};

#endif //LAB1_GENERATOR_H