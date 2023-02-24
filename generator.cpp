#include "generator.h"

#include <stdexcept>
#include <algorithm>
#include <iostream>

generator::generator(match *m, std::string input): matcher(m), input(std::move(input)){}

void generator::get_result() {
    if(matcher != nullptr){
        matcher->evaluate(input.begin(), input.end());
        return;
    }
    throw std::invalid_argument("Couldn't resolve match");
}

//<match> := <level>
std::string match::evaluate(IT it, IT last) {
    for(; it < last; it++){
        std::string s = children[0]->evaluate(it, last);
        if(!s.empty()){
            IT start = dynamic_cast<level*>(children[0])->start;
            for(;it != start; it++){ std::cout << *it; }

            std::cout << "\x1B[32m" + s + "\033[0m";
            it = it + (int)s.size() - 1;
            continue;
        }
        std::cout << *it;
    }
    return {};
}

void traverse_tree(const std::vector<base*>& children, std::vector<std::vector<base*>>& levels){
    if(dynamic_cast<subexpression*>(children[0])){
        levels.push_back(children[0]->children);
        traverse_tree(children[0]->children, levels);
    }else if(!dynamic_cast<string*>(children[0])){
        traverse_tree(children[0]->children, levels);
    }
    if(children.size() > 1){
        traverse_tree(children[1]->children, levels);
    }
}

//<level> := <expressions>\O{<digit>}
std::string level::evaluate(IT it, IT last) {
    if(!children[0]->evaluate(it, last).empty()) {
        std::vector<std::vector<base*>> nodes;
        nodes.push_back(children);
        traverse_tree(children, nodes);

        int index = *digit - '0';
        if(nodes.size() - 1 < index){ throw std::invalid_argument("Couldn't resolve levels"); }

        for(int i = 0; i < index + 1; i++) {
            for (; it != last; it++) {
                if (!nodes[i][0]->evaluate(it, last).empty()) {
                    break;
                }
            }
        }
        start = it;
        return nodes[index][0]->evaluate(it, last);
    }
    return {};
}

//<expressions> := <expression> | <expression><expressions>
std::string expressions::evaluate(IT it, IT last) {
    if(dynamic_cast<expression*>(children[0])) {
        std::string result = children[0]->evaluate(it, last);
        if(!result.empty()){
            if(ignore){ children[0]->ignore = true; }
            if(children.size() > 1){
                std::string next_result = children[1]->evaluate(it + (int)result.size(), last);
                if(next_result.empty()){
                    return {};
                }
                return result + next_result;
            }
            return result;
        }
        return {};
    }
    throw std::runtime_error("Couldn't evaluate expressions");
}

//<subexpression> := (<expressions>)
std::string subexpression::evaluate(IT it, IT last) {
    if(dynamic_cast<expressions*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate subexpression");
}

//<ignore> := <operand>\I
std::string ignore::evaluate(IT it, IT last) {
    for(auto child : children){
        child->ignore = true;
        return child->evaluate(it, last);
    }
    return {};
}

//<expression> := <operand> | <either> | <many> | <count>
std::string expression::evaluate(IT it, IT last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    throw std::invalid_argument("Couldn't evaluate expression");
}

//<either> := <operand>+<operand> | <operand>+<either>
std::string either::evaluate(IT it, IT last){
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        if(!child->evaluate(it, last).empty()){
            return child->evaluate(it, last);
        }
    }
    return {};
}

//<many> := <operand>*
std::string many::evaluate(IT it, IT last) {
    if(ignore){ children[0]->ignore = true; }
    if(it < last) {
        std::string result = children[0]->evaluate(it, last);
        if (!result.empty()) {
            std::string next_result = children[0]->evaluate(it + (int)result.size(), last);
            if (!next_result.empty()) {
                return result + evaluate(it + (int)result.size(), last);
            }
            return result;
        }
    }
    return {};
}

//<count> := <operand>{<digit>}
std::string count::evaluate(IT it, IT last) {
    std::string eval;
    std::string result = children[0]->evaluate(it, last);
    for(int i = 0; i < *digit - '0'; i++){
        if (children[0]->evaluate(it + i * (int)result.size(), last).empty()) {
            return {};
        }
        eval += children[0]->evaluate(it + i * (int)result.size(), last);
    }
    return eval;
}

//<operand> := <string> | <subexpression>
std::string operand::evaluate(IT it, IT last) {
    if(dynamic_cast<string*>(children[0]) || dynamic_cast<subexpression*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate operand");
}

//<string> := <letter> | <wildcard> | <letter><string> | <wildcard><string>
std::string string::evaluate(IT it, IT last) {
    if(dynamic_cast<wildcard*>(children[0]) || dynamic_cast<letter*>(children[0])) {
        if(ignore){ children[0]->ignore = true; }
        std::string result = children[0]->evaluate(it, last);
        if (children.size() > 1) {
            if(dynamic_cast<string*>(children[1])) {
                if(ignore){ children[1]->ignore = true; }

                std::string next_result = children[1]->evaluate(it + 1, it + 2);
                if (result.empty() || next_result.empty()) {
                    return {};
                }
                return result + next_result;
            }
        }
        return result;
    }
    throw std::runtime_error("Couldn't evaluate string");
}

std::string letter::evaluate(IT it, IT last) {
    if(ignore){
        if(*it == std::tolower(*letter) || *it == std::toupper(*letter)){
            return std::string{ *it };
        }
    }else{
        if (*it == *letter) {
            return std::string{ *it };
        }
    }
    return {};
}

std::string wildcard::evaluate(IT it, IT last) {
    return std::string{ *it };
}