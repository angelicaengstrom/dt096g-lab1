#include "generator.h"

#include <stdexcept>
#include <algorithm>
#include <iostream>

generator::generator(match *m, std::string input): matcher(m), input(std::move(input)){}

void generator::get_result() {
    if(matcher != nullptr){
        IT begin = input.begin();
        IT end = input.end();
        for(IT it = begin; it < end; it++, begin++) {
            if(matcher->evaluate(it, end)){
                for(; begin != it; begin++){
                    std::cout << "\x1b[44m" << *begin << "\033[0m";
                }
                std::cout << "\x1b[44m" << *it << "\033[0m";
                continue;
            }
            std::cout << *begin;
        }
        return;
    }
    throw std::invalid_argument("Couldn't resolve match");
}

//<match> := <level>
bool match::evaluate(IT &it, IT &last) {
    return children[0]->evaluate(it, last);
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
bool level::evaluate(IT &it, IT &last) {
    IT start = it;

    std::vector<std::vector<base*>> nodes;
    nodes.push_back(children);
    traverse_tree(children, nodes);

    int index = *digit - '0';
    if(nodes.size() - 1 < index){ throw std::invalid_argument("Couldn't resolve levels"); }

    if(!nodes[index][0]->evaluate(it, last)){
        it = start;
        return false;
    }
    return true;
}

//<expressions> := <expression> | <expression><expressions>
bool expressions::evaluate(IT &it, IT &last) {
    if(dynamic_cast<expression*>(children[0])) {
        if(ignore){ children[0]->ignore = true; }
        if(children[0]->evaluate(it, last)) {
            if (children.size() > 1) {
                if (!children[1]->evaluate(++it, last)) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    throw std::runtime_error("Couldn't evaluate expressions");
}

//<subexpression> := (<expressions>)
bool subexpression::evaluate(IT &it, IT &last) {
    if(dynamic_cast<expressions*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate subexpression");
}

//<ignore> := <operand>\I
bool ignore::evaluate(IT &it, IT &last) {
    for(auto child : children){
        child->ignore = true;
        return child->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate ignore");
}

//<expression> := <operand> | <either> | <many> | <count>
bool expression::evaluate(IT &it, IT &last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    throw std::invalid_argument("Couldn't evaluate expression");
}

//<either> := <operand>+<operand> | <operand>+<either>
bool either::evaluate(IT &it, IT &last){
    IT start = it;
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        if(child->evaluate(start, last)){
            it = start;
            return true;
        }
    }
    return false;
}

//<many> := <operand>*
bool many::evaluate(IT &it, IT &last) {
    if(ignore){ children[0]->ignore = true; }
    if(it < last) {
        if (children[0]->evaluate(it, last)) {
            IT stop = it;
            if(!evaluate(++it, last)){
                it = stop;
            }
            return true;
        }
    }
    return false;
}

//<count> := <operand>{<digit>}
bool count::evaluate(IT &it, IT &last){
    for(int i = 0; i < *digit - '0'; i++, it++){
        if (!children[0]->evaluate(it, last)) {
            return false;
        }
    }
    return true;
}

//<operand> := <string> | <subexpression>
bool operand::evaluate(IT &it, IT &last) {
    if(dynamic_cast<string*>(children[0]) || dynamic_cast<subexpression*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate operand");
}

//<string> := <letter> | <wildcard> | <letter><string> | <wildcard><string>
bool string::evaluate(IT &it, IT &last) {
    if(dynamic_cast<wildcard*>(children[0]) || dynamic_cast<letter*>(children[0])) {
        if(ignore){ children[0]->ignore = true; }
        if(children[0]->evaluate(it, last)) {
            if (children.size() > 1) {
                if (dynamic_cast<string *>(children[1])) {
                    if (ignore) { children[1]->ignore = true; }

                    if (!children[1]->evaluate(++it, last)) {
                        return false;
                    }
                }
            }
            return true;
        }
        return false;
    }
    throw std::runtime_error("Couldn't evaluate string");
}

bool letter::evaluate(IT &it, IT &last) {
    if(ignore){
        return *it == std::tolower(*letter) || *it == std::toupper(*letter);
    }
    return *it == *letter;
}

bool wildcard::evaluate(IT &it, IT &last) {
    return true;
}