//
// Created by angel on 2023-02-02.
//

#include "parser.h"

parser::parser(IT first, IT last):it(first), last(last) {
    //matcher = parse_match();
    set_match(parse_match());
}

match *parser::get_match() const {
    return matcher;
}

void parser::set_match(match *m) {
    matcher = m;
}

//<match> := <level>
match *parser::parse_match() {
    auto id_level = parse_level();
    if(!id_level){
        return nullptr;
    }
    auto result = new match();
    result->children.push_back(id_level);
    return result;
}

//<level> := <expressions>\O{<digit>}
level *parser::parse_level() {
    auto id_expressions = parse_expressions();
    if(!id_expressions){
        return nullptr;
    }
    auto lvl = new level();
    lvl->children.push_back(id_expressions);
    if(lex.get_current(it, last) == lexer::LEVEL_OP){
        it = it + 2;
        if(lex.get_current(it, last) == lexer::LBRACKET){
            it++;
            if(lex.get_current(it, last) == lexer::DIGIT){
                IT d = it;
                it++;
                if(lex.get_current(it, last) == lexer::RBRACKET){
                    lvl->digit = d;
                    return lvl;
                }
            }
        }
        throw std::invalid_argument("Couldn't resolve level");
    }else{
        lvl->digit = IT((char*)"0");
    }
    return lvl;
}

//<expressions> := <expression> | <expression><expressions> | <greedy>
expressions *parser::parse_expressions() {
    auto id_expression = parse_expression();
    if(!id_expression){
        return nullptr;
    }
    auto result = new expressions();
    auto m = dynamic_cast<many*>(id_expression->children[0]);
    if(m) {
        auto id_greedy = parse_greedy(m);
        if (id_greedy) {
            result->children.push_back(id_greedy);
            return result;
        }
    }
    result->children.push_back(id_expression);

    auto new_expression = parse_expressions();
    if(new_expression){
        result->children.push_back(new_expression);
    }
    return result;
}

//<greedy> := <many><expressions>
greedy *parser::parse_greedy(many* m) {
    auto id_expressions = parse_expressions();
    if(!id_expressions){
        return nullptr;
    }
    auto result = new greedy();
    result->children.push_back(m);
    result->children.push_back(id_expressions);
    return result;
}

//<subexpression> := (<expressions>)
subexpression *parser::parse_subexpression() {
    if(lex.get_current(it, last) == lexer::LPAREN) {
        it++;
        auto id_expressions = parse_expressions();
        if(!id_expressions){
            throw std::invalid_argument("Couldn't resolve expressions");
        }
        if(lex.get_current(it, last) == lexer::RPAREN){
            auto sub = new subexpression();
            sub->children.push_back(id_expressions);
            it++;
            return sub;
        }
        throw std::invalid_argument("Couldn't resolve subexpression");
    }
    return nullptr;
}

//<expression> := <operand> | <either> | <many> | <count>
expression *parser::parse_expression() {
    auto id_operand = parse_operand();
    if(!id_operand){
        return nullptr;
    }
    auto result = new expression();
    auto id_either = parse_either(id_operand);
    if(id_either){
        result->children.push_back(id_either);
        return result;
    }
    auto id_many = parse_many(id_operand);
    if(id_many){
        result->children.push_back(id_many);
        return result;
    }
    auto id_count = parse_count(id_operand);
    if(id_count){
        result->children.push_back(id_count);
        return result;
    }
    result->children.push_back(id_operand);
    return result;
}

//<either> := <operand>+<operand> | <operand>+<either>
either *parser::parse_either(operand* op) {
    if(lex.get_current(it, last) == lexer::EITHER_OP){
        it++;
        auto id_operand = parse_operand();
        if(!id_operand){
            throw std::invalid_argument("Couldn't resolve either '+'");
        }
        auto e = new either();

        e->children.push_back(op);
        e->children.push_back(id_operand);

        auto new_either = parse_either(id_operand);
        if(new_either){
            e->children.push_back(new_either);
        }
        return e;
    }
    return nullptr;
}

//<many> := <operand>*
many *parser::parse_many(operand* op) {
    if(lex.get_current(it, last) == lexer::MANY_OP){
        auto m = new many();
        m->children.push_back(op);
        it++;
        return m;
    }
    return nullptr;
}

//<count> := <operand>{<digit>}
count *parser::parse_count(operand* op) {
    if(lex.get_current(it, last) == lexer::LBRACKET){
        it++;
        if(lex.get_current(it, last) == lexer::DIGIT){
            IT d = it;
            it++;
            if(lex.get_current(it, last) == lexer::RBRACKET){
                auto c = new count();
                c->digit = d;
                c->children.push_back(op);
                it++;
                return c;
            }
        }
        throw std::invalid_argument("Couldn't resolve count");
    }
    return nullptr;
}

//<ignore> := <subexpression>\I | <string>\I
ignore *parser::parse_ignore(string* s) {
    if(lex.get_current(it, last) == lexer::IGNORE_OP){
        it = it + 2;
        auto i = new ignore();
        i->children.push_back(s);
        return i;
    }
    return nullptr;
}
//<ignore> := <subexpression>\I | <string>\I
ignore *parser::parse_ignore(subexpression* sub) {
    if(lex.get_current(it, last) == lexer::IGNORE_OP){
        //it = it + 2;
        auto i = new ignore();
        i->children.push_back(sub);
        return i;
    }
    return nullptr;
}

//<operand> := <ignore> | <subexpression> | <string>
operand *parser::parse_operand() {
    auto id_string = parse_string();
    if(id_string){
        auto op = new operand();
        auto id_ignore = parse_ignore(id_string);
        if(id_ignore){
            op->children.push_back(id_ignore);
            return op;
        }
        op->children.push_back(id_string);
        return op;
    }
    auto id_subexpression = parse_subexpression();
    if(id_subexpression){
        auto op = new operand();
        auto id_ignore = parse_ignore(id_subexpression);
        if(id_ignore){
            op->children.push_back(id_ignore);
            return op;
        }
        op->children.push_back(id_subexpression);
        return op;
    }
    return nullptr;
}

//<string> := <letter> | <wildcard> | <letter><string> | <wildcard><string>
string *parser::parse_string() {
    if(lex.get_current(it, last) == lexer::WILDCARD || lex.get_current(it, last) == lexer::LETTER){
        auto s = new string();
        if(lex.get_current(it, last) == lexer::LETTER){
            auto l = new letter();
            l->letter = it;
            s->children.push_back(l);
        }else{
            s->children.push_back(new wildcard());
        }
        it++;

        auto new_string = parse_string();
        if(new_string) {
            s->children.push_back(new_string);
        }
        return s;
    }
    return nullptr;
}
