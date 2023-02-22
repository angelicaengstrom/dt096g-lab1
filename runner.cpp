//
// Created by angel on 2023-02-11.
//

#include "runner.h"

#include <stdexcept>
#include <algorithm>
#include <iostream>

runner::runner(match *m, std::string input): matcher(m), input(std::move(input)){}

std::vector<std::string> runner::get_result() {
    if(matcher != nullptr){
        matcher->evaluate(input.begin(), input.end());
        return matcher->result;
    }
    throw std::invalid_argument("Argument not found");
}

std::string string::evaluate(std::string::iterator it, std::string::iterator last) {
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
    /*
    if(dynamic_cast<letter*>(children[0])){
        if(!children[0]->evaluate(it, last).empty()) {
            if(children.size() > 1) {
                if(children[1]->evaluate(it + 1, it + 2).empty()){
                    return {};
                }
                return children[0]->evaluate(it, last) + children[1]->evaluate(it + 1, last);
            }
            return children[0]->evaluate(it, last);
        }
    }else if(dynamic_cast<wildcard*>(children[0])){
        if(children.size() > 1){
            if(children[1]->evaluate(it + 1, it + 2).empty()){
                return {};
            }
            return children[0]->evaluate(it, last) + children[1]->evaluate(it + 1, last);
        }
        return children[0]->evaluate(it, last);
    }
    return {};*/
}

std::string letter::evaluate(std::string::iterator it, std::string::iterator last) {
    if(ignore){
        if(*it == std::tolower(*letter) || *it == std::toupper(*letter)){
            return std::string(1, *it);
        }
    }
    if(*it == *letter){
        return std::string(1, *it);
    }
    return {};
}

std::string expression::evaluate(std::string::iterator it, std::string::iterator last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    throw std::invalid_argument("Couldn't resolve expression");
}

std::string match::evaluate(std::string::iterator it, std::string::iterator last) {
    for(; it < last; it++){
        std::string s = children[0]->evaluate(it, last);
        if(!s.empty()){
            auto lvl = dynamic_cast<level*>(children[0]);
            while(it != lvl->start){
                std::cout << *it;
                it++;
            }
            result.push_back(s);
            std::cout << "\x1B[32m" + s + "\033[0m";
            //std::cout << "[" << s << "]";
            it = it + s.size() - 1;
            continue;
        }
        std::cout << *it;
    }
    return {};
}

std::string wildcard::evaluate(std::string::iterator it, std::string::iterator last) {
    return std::string(1, *it);
}

std::string either::evaluate(std::string::iterator it, std::string::iterator last){
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

std::string many::evaluate(std::string::iterator it, std::string::iterator last) {
    if(ignore){ children[0]->ignore = true; }
    if(it < last) {
        if (!children[0]->evaluate(it, last).empty()) {
            /*Arbetar med den sista bokstaven*/
            /*std::vector<base*> l;
            find_last(children[0]->children, l);
            std::string eval = children[0]->evaluate(it, last);
            it = it + children[0]->evaluate(it, last).size();
            while(!l[0]->evaluate(it, last).empty()){
                eval += l[0]->evaluate(it, last);
                it++;
            }
            return eval;*/

            /*Arbetar med hela ordet*/
            if (!children[0]->evaluate(it + children[0]->evaluate(it, last).size(), last).empty()) {
                return children[0]->evaluate(it, last) + evaluate(it + children[0]->evaluate(it, last).size(), last);
            }
            return children[0]->evaluate(it, last);
        }
    }
    return {};
}

std::string subexpression::evaluate(std::string::iterator it, std::string::iterator last) {
    for(auto child : children){
        if(ignore){ child->ignore = true; }
        return child->evaluate(it, last);
    }
    return {};
}

std::string expressions::evaluate(std::string::iterator it, std::string::iterator last) {
    if(ignore){ children[0]->ignore = true; }
    if(dynamic_cast<expression*>(children[0]) || dynamic_cast<subexpression*>(children[0])) {
        if(!children[0]->evaluate(it, last).empty()){
            if(children.size() > 1){
                if(children[1]->evaluate(it + children[0]->evaluate(it, last).size(), last).empty()){
                    return std::string();
                }
                return children[0]->evaluate(it, last) + children[1]->evaluate(it + children[0]->evaluate(it, last).size(), last);
            }
            return children[0]->evaluate(it, last);
        }
        return {};
    }
    throw std::invalid_argument("Couldn't resolve expressions");
}

std::string count::evaluate(std::string::iterator it, std::string::iterator last) {
    std::string eval;

    for(int i = 0; i < *digit - '0'; i++){
        if (children[0]->evaluate(it + i * children[0]->evaluate(it, last).size(), last).empty()) {
            return {};
        }
        eval += children[0]->evaluate(it + i * children[0]->evaluate(it, last).size(), last);
    }
    return eval;
}

void traverse_tree(const std::vector<base*>& children, std::vector<std::vector<base*>>& levels){
    if(dynamic_cast<subexpression*>(children[0])){
        levels.push_back(children[0]->children);
        traverse_tree(children[0]->children, levels);
    }else if(dynamic_cast<expressions*>(children[0]) || dynamic_cast<ignore*>(children[0])){
        traverse_tree(children[0]->children, levels);
    }
    if(children.size() > 1){
        traverse_tree(children[1]->children, levels);
    }
}

std::string level::evaluate(std::string::iterator it, std::string::iterator last) {
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

std::string ignore::evaluate(std::string::iterator it, std::string::iterator last) {
    for(auto child : children){
        child->ignore = true;
        return child->evaluate(it, last);
    }
    return {};
}
