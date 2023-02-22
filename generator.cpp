//
// Created by angel on 2023-02-11.
//

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

std::string level::evaluate(IT it, IT last) {
    if(!children[0]->evaluate(it, last).empty()) {
        std::vector<std::vector<base*>> nodes;
        nodes.push_back(children);
        traverse_tree(children, nodes);
        if(nodes.size() - 1 < *digit - '0'){
            throw std::invalid_argument("Couldn't resolve levels");
        }
        for(int i = 0; i < *digit - '0' + 1; i++) {
            for (; it != last; it++) {
                if (!nodes[i][0]->evaluate(it, last).empty()) {
                    break;
                }
            }
        }
        start = it;
        return nodes[*digit - '0'][0]->evaluate(it, last);
    }
    return {};
}

std::string string::evaluate(IT it, IT last) {
    if(ignore){ children[0]->ignore = true; }
    if(children.size() > 1){ if(ignore){ children[1]->ignore = true; } }
    if(dynamic_cast<letter*>(children[0])){
        if(children[0]->evaluate(it, last).empty()){ return {}; }
    }

    if(dynamic_cast<wildcard*>(children[0]) || dynamic_cast<letter*>(children[0])) {
        if (children.size() > 1) {
            if(dynamic_cast<string*>(children[1])) {
                if (children[1]->evaluate(it + 1, it + 2).empty()) {
                    return {};
                }
                return children[0]->evaluate(it, last) + children[1]->evaluate(it + 1, it + 2);
            }
        }
        return children[0]->evaluate(it, last);
    }
    return {};
}

std::string letter::evaluate(IT it, IT last) {
    if(ignore){
        if(*it == std::tolower(*letter) || *it == std::toupper(*letter)){
            return std::string{ *it };
        }
    }
    if(*it == *letter){
        return std::string{ *it };
    }
    return {};
}

std::string expression::evaluate(IT it, IT last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    throw std::invalid_argument("Couldn't resolve expression");
}

std::string wildcard::evaluate(IT it, IT last) {
    return std::string{ *it };
}

std::string either::evaluate(IT it, IT last){
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        if(!child->evaluate(it, last).empty()){
            return child->evaluate(it, last);
        }
    }
    return {};
}

void find_last(const std::vector<base*>& children, std::vector<base*>& l){
    if(children.size() < 2){
        l = children;
    }else{
        find_last(children[1]->children, l);
    }
}

std::string many::evaluate(IT it, IT last) {
    if(ignore){ children[0]->ignore = true; }
    if(it < last) {
        if (!children[0]->evaluate(it, last).empty()) {
            if (!children[0]->evaluate(it + (int)children[0]->evaluate(it, last).size(), last).empty()) {
                return children[0]->evaluate(it, last) + evaluate(it + (int)children[0]->evaluate(it, last).size(), last);
            }
            return children[0]->evaluate(it, last);
        }
    }
    return {};
}

std::string subexpression::evaluate(IT it, IT last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    return {};
}

std::string expressions::evaluate(IT it, IT last) {
    if(ignore){ children[0]->ignore = true; }
    if(dynamic_cast<expression*>(children[0]) || dynamic_cast<subexpression*>(children[0])) {
        if(!children[0]->evaluate(it, last).empty()){
            if(children.size() > 1){
                if(children[1]->evaluate(it + (int)children[0]->evaluate(it, last).size(), last).empty()){
                    return {};
                }
                return children[0]->evaluate(it, last) + children[1]->evaluate(it + (int)children[0]->evaluate(it, last).size(), last);
            }
            return children[0]->evaluate(it, last);
        }
        return {};
    }
    throw std::invalid_argument("Couldn't resolve expressions");
}

std::string count::evaluate(IT it, IT last) {
    std::string eval;
    for(int i = 0; i < *digit - '0'; i++){
        if (children[0]->evaluate(it + i * (int)children[0]->evaluate(it, last).size(), last).empty()) {
            return {};
        }
        eval += children[0]->evaluate(it + i * (int)children[0]->evaluate(it, last).size(), last);
    }
    return eval;
}

std::string ignore::evaluate(IT it, IT last) {
    for(auto child : children){
        child->ignore = true;
        return child->evaluate(it, last);
    }
    return {};
}

std::string operand::evaluate(IT it, IT last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    return {};
}

std::string between::evaluate(IT it, IT last) {
    return std::__cxx11::string();
}
