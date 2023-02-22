//
// Created by angel on 2023-02-11.
//

#ifndef LAB1_RUNNER_H
#define LAB1_RUNNER_H

#include <vector>
#include <string>

struct base{
    virtual std::string evaluate(std::string::iterator it, std::string::iterator last) = 0;
    std::vector<base*> children;
    bool ignore = false;
};

struct match:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
    std::vector<std::string> result;
};

struct level:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
    std::string::iterator digit;
    std::string::iterator start;
};

struct expressions:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct ignore:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct expression:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct subexpression:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct either:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct count:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
    std::string::iterator digit;
};

struct many:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct string:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

struct letter:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
    std::string::iterator letter;
};

struct wildcard:base{
    std::string evaluate(std::string::iterator it, std::string::iterator last) override;
};

class runner {
private:
    match *matcher;
    std::string input;
public:
    runner(match *m, std::string input);
    std::vector<std::string> get_result();
};

#endif //LAB1_RUNNER_H
