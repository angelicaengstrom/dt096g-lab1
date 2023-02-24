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
bool match::evaluate(IT it, IT last) {
    for(; it < last; it++){
        if(children[0]->evaluate(it, last)){
            //IT start = dynamic_cast<level*>(children[0])->start;
            //for(;it != start; it++){ std::cout << *it; }
            std::cout << "\x1B[32m" << *it << "\033[0m";

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
bool level::evaluate(IT it, IT last) {
    if(children[0]->evaluate(it, last)) {
        std::vector<std::vector<base*>> nodes;
        nodes.push_back(children);
        traverse_tree(children, nodes);

        int index = *digit - '0';
        if(nodes.size() - 1 < index){ throw std::invalid_argument("Couldn't resolve levels"); }

        for(int i = 0; i < index + 1; i++) {
            for (; it != last; it++) {
                if (nodes[i][0]->evaluate(it, last)) {
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
bool expressions::evaluate(IT it, IT last) {
    if(dynamic_cast<expression*>(children[0])) {
        if(ignore){ children[0]->ignore = true; }
        if(children.size() > 1){
            return children[1]->evaluate(it, last);
        }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate expressions");
}

//<subexpression> := (<expressions>)
bool subexpression::evaluate(IT it, IT last) {
    if(dynamic_cast<expressions*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate subexpression");
}

//<ignore> := <operand>\I
bool ignore::evaluate(IT it, IT last) {
    for(auto child : children){
        child->ignore = true;
        return child->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate ignore");
}

//<expression> := <operand> | <either> | <many> | <count>
bool expression::evaluate(IT it, IT last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    throw std::invalid_argument("Couldn't evaluate expression");
}

//<either> := <operand>+<operand> | <operand>+<either>
bool either::evaluate(IT it, IT last){
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        if(child->evaluate(it, last)){
            return child->evaluate(it, last);
        }
    }
    return false;
}

//<many> := <operand>*
bool many::evaluate(IT it, IT last) {
    if(ignore){ children[0]->ignore = true; }
    if(it < last) {
        if (children[0]->evaluate(it, last)) {
            if (children[0]->evaluate(it + 1, last)) {
                return evaluate(it + 1, last);
            }
            return children[0]->evaluate(it + 1, last);
        }
    }
    return {};
}

//<count> := <operand>{<digit>}
bool count::evaluate(IT it, IT last) {
    for(int i = 0; i < *digit - '0'; i++){
        if (!children[0]->evaluate(it + i * 1, last)) {
            return false;
        }
    }
    return true;
}

//<operand> := <string> | <subexpression>
bool operand::evaluate(IT it, IT last) {
    if(dynamic_cast<string*>(children[0]) || dynamic_cast<subexpression*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate operand");
}

//<string> := <letter> | <wildcard> | <letter><string> | <wildcard><string>
bool string::evaluate(IT it, IT last) {
    if(dynamic_cast<wildcard*>(children[0]) || dynamic_cast<letter*>(children[0])) {
        if(ignore){ children[0]->ignore = true; }
        if (children.size() > 1) {
            if(dynamic_cast<string*>(children[1])) {
                if(ignore){ children[1]->ignore = true; }

                if (!children[0]->evaluate(it, last) || !children[1]->evaluate(it + 1, it + 2)) {
                    return false;
                }
                return children[1]->evaluate(it + 1, it + 2);
            }
        }
        return children[0]->evaluate(it, last);
    }
    throw std::runtime_error("Couldn't evaluate string");
}

bool letter::evaluate(IT it, IT last) {
    if(ignore){
        return *it == std::tolower(*letter) || *it == std::toupper(*letter);
    }
    return *it == *letter;
}

bool wildcard::evaluate(IT it, IT last) {
    return true;
}