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

IT lvl_start;
IT lvl_stop;

generator::generator(match *m, std::string input): matcher(m), input(std::move(input)){}

std::ostream& operator<<(std::ostream& out, const text& t){
    out << t.get_text();
    return out;
}

void generator::get_result() {
    if(matcher != nullptr){
        IT begin = input.begin();
        IT end = input.end();
        for(IT it = begin; it < end; it++, begin++) {
            if(matcher->evaluate(it, end)){
                for(; begin < lvl_start; begin++){ std::cout << *begin; }
                for(; lvl_start <= lvl_stop; lvl_start++, begin++){
                    t.set_text(lvl_start);
                    std::cout << t;
                }
                t.update();
                lvl_stop++;
                for(; lvl_stop < std::next(it); lvl_stop++){ std::cout << *lvl_stop; }
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
    if(children[0]->evaluate(it, last)){
        return true;
    }
    return false;
}

//<level> := <expressions>\O{<digit>}
bool level::evaluate(IT &it, IT &last) {
    lvl = *digit - '0';
    if(lvl == 0){ lvl_start = it; }
    lvl_stop = last;
    IT start = it;
    if(children[0]->evaluate(it, last)){
        if(lvl == 0){ lvl_stop = it; }
        current_lvl = 0;
        return true;
    }
    it = start;
    return false;
}

//<expressions> := <expression> | <expression><expressions>
bool expressions::evaluate(IT &it, IT &last) {
    if(dynamic_cast<expression*>(children[0]) || dynamic_cast<greedy*>(children[0])) {
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

//<greedy> := <many><expressions> | <many><expressions>)<expressions> | <many>)<expressions> | (<many>) | <many><expressions>)
bool greedy::evaluate(IT &it, IT &last) {
    IT start = it;
    if(children[0]->evaluate(it, last)){
        if(children.size() > 1) {
            if (std::next(it) < last){ return children[1]->evaluate(++it, last); }
            for(IT temp = it; temp != start; temp--){
                if(current_lvl == lvl){ lvl_stop = temp; }
                if(children[1] == nullptr){ return true; }
                if(children[1]->evaluate(it, last)){
                    if(children.size() > 2){
                        if(current_lvl == lvl){ lvl_stop = it; }
                        if(children[2] == nullptr){ return true; }
                        if(children[2]->evaluate(++it, last)){ return true; }
                    }else{
                        return true;
                    }
                }
                it = temp;
            }
            return false;
        }else{
            if(current_lvl == lvl){ lvl_stop = it; }
            return true;
        }
    }
    return false;
}

//<subexpression> := (<expressions>)
bool subexpression::evaluate(IT &it, IT &last) {
    current_lvl++;
    if(lvl == current_lvl){ lvl_start = it; }
    if(dynamic_cast<expressions*>(children[0])){
        if(ignore){ children[0]->ignore = true; }
        if(children[0]->evaluate(it, last)) {
            if (lvl == current_lvl && lvl_stop == last) { lvl_stop = it; }
            return true;
        }
        current_lvl--;
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
    if(*digit - '0' == 0){ return false; }
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

