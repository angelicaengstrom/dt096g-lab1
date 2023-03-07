#include "generator.h"

#include <stdexcept>
#include <algorithm>
#include <iostream>

/* WARNING: the code that follows will make you cry:
 *          a safety pig is provided below for your benefit
 *
 *
 *
 *                          _
 _._ _..._ .-',     _.._(`))
'-. `     '  /-._.-'    ',/
   )         \            '.
  / _    _    |             \
 |  a    a    /              |
 \   .-.                     ;
  '-('' ).-'       ,'       ;
     '-;           |      .'
        \           \    /
        | 7  .__  _.-\   \
        | |  |  ``/  /`  /
       /,_|  |   /,_/   /
          /,_/      '`-'
 */
IT sub_start;
IT sub_stop;
int curr;
int lvl;

generator::generator(match *m, std::string input): matcher(m), input(std::move(input)){}

std::ostream& operator<<(std::ostream& out, const text& t){
    out << t.get_text();
    return out;
}

void generator::get_result() {
    lvl = 0;
    if(matcher != nullptr){
        IT begin = input.begin();
        IT end = input.end();
        for(IT it = begin; it < end; it++, begin++) {
            curr = 0;
            if(matcher->evaluate(it, end)){
                if(lvl > 0) {
                    for (; begin < sub_start; begin++) {
                        std::cout << *begin;
                    }
                    for (; sub_start != sub_stop; sub_start++, begin++) {
                        t.set_text(sub_start);
                        std::cout << t;
                    }
                    for (; begin != it; begin++) {
                        std::cout << *begin;
                    }
                    std::cout << *it;
                    t.update();
                }else{
                    for (; begin != it; begin++) {
                        t.set_text(begin);
                        std::cout << t;
                    }
                    t.set_text(it);
                    std::cout << t;
                    t.update();
                }
                continue;
            }
            std::cout << *it;
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
    lvl = *digit - '0';
    IT start = it;
    if(children[0]->evaluate(it, last)){
        return true;
    }
    it = start;
    return false;
}

//<expressions> := <expression> | <expression><expressions>
bool expressions::evaluate(IT &it, IT &last) {
    if(dynamic_cast<expression*>(children[0]) /*|| dynamic_cast<greedy*>(children[0])*/) {
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

/*bool greedy::evaluate(IT &it, IT &last) {
    IT start = it;
    if(children[0]->evaluate(it, last)){
        if (std::next(it) < last){ return children[1]->evaluate(++it, last); }

        for(IT temp = it; start != it; it--){
            if(children[1]->evaluate(temp, last)){
                it = temp;
                return true;
            }
            temp = it;
        }
    }
    return false;
}*/

//<subexpression> := (<expressions>)
bool subexpression::evaluate(IT &it, IT &last) {
    curr++;
    if(lvl == curr){
        sub_start = it;
    }
    if(dynamic_cast<expressions*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        if(children[0]->evaluate(it, last)) {
            if (lvl == curr) {
                sub_stop = std::next(it);
            }
            return true;
        }
        return false;
    }
    throw std::runtime_error("Couldn't evaluate subexpression");
}

//<ignore> := <subexpression>\I | <string>\I
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
    IT stop;
    for(int i = 0; i < *digit - '0'; i++){
        if (!children[0]->evaluate(it, last)) {
            return false;
        }
        stop = it++;
    }
    it = stop;
    return true;
}

//<operand> := <string> | <subexpression>
bool operand::evaluate(IT &it, IT &last) {
    if(ignore){ children[0]->ignore = true; }
    return children[0]->evaluate(it, last);
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
