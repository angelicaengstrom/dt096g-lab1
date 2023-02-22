//
// Created by angel on 2023-02-02.
//

#include "parser.h"

parser::parser(std::string::iterator first, std::string::iterator last):it(first), last(last) {
    matcher = parse_match();
    //set_match(parse_match());
}

match *parser::get_match() const {
    return matcher;
}

void parser::set_match(match *m) {
    matcher = m;
}

//<match> := <level>
match *parser::parse_match() {
    auto id_expressions = parse_expressions();
    if(!id_expressions){
        return nullptr;
    }
    auto result = new match();
    auto id_level = parse_level();
    if(!id_level){
        id_level = new level();
        id_level->digit = std::string::iterator((char*)"0");
    }
    id_level->children.push_back(id_expressions);
    result->children.push_back(id_level);
    return result;
}

//<expression> := <string> | <either> | <many> | <count>
expression *parser::parse_expression() {
    auto id_string = parse_string();
    if(!id_string){
        return nullptr;
    }
    auto result = new expression();
    auto id_either = parse_either();
    if(id_either){
        id_either->children.push_back(id_string);
        result->children.push_back(id_either);
        return result;
    }
    auto id_many = parse_many();
    if(id_many){
        id_many->children.push_back(id_string);
        id_either = parse_either();
        if(id_either){
            id_either->children.push_back(id_many);
            result->children.push_back(id_either);
            return result;
        }
        result->children.push_back(id_many);
        return result;
    }
    auto id_count = parse_count();
    if(id_count){
        id_count->children.push_back(id_string);
        auto id_either = parse_either();
        if(id_either){
            id_either->children.push_back(id_count);
            result->children.push_back(id_either);
            return result;
        }
        result->children.push_back(id_count);
        return result;
    }
    result->children.push_back(id_string);
    return result;
}

//<string> := <letter> | <wildcard> | <letter><string> | <wildcard><string>
string *parser::parse_string() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::LETTER){
        auto l = new letter();
        l->letter = it;

        auto s = new string();
        s->children.push_back(l);

        it++;

        auto new_string = parse_string();
        if(new_string) {
            s->children.push_back(new_string);
        }
        return s;
    }else if(lex.get_current_token() == lexer::WILDCARD){
        auto w = new wildcard();

        auto s = new string();
        s->children.push_back(w);

        it++;

        auto new_string = parse_string();
        if(new_string) {
            s->children.push_back(new_string);
        }
        return s;
    }
    return nullptr;
}

//<either> := <string>+<string> | <string>+<either> | <string>+<many> | <many>+<string> | <many>+<many> | <count>+<string> | <string>+<count> | <count>+<count>
either *parser::parse_either() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::EITHER_OP){
        it++;
        auto id_string = parse_string();
        if(!id_string){
            throw std::invalid_argument("Couldn't resolve either '+'");
        }
        auto e = new either();

        auto id_many = parse_many();
        auto id_count = parse_count();
        if(id_many){
            id_many->children.push_back(id_string);
            e->children.push_back(id_many);
        }else if(id_count){
            id_count->children.push_back(id_string);
            e->children.push_back(id_count);
        }else{
            e->children.push_back(id_string);
        }

        auto new_either = parse_either();
        if(new_either){
            e->children.push_back(new_either);
        }
        return e;
    }
    return nullptr;
}

//<many> := <string>*
many *parser::parse_many() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::MANY_OP){
        auto m = new many();
        it++;
        return m;
    }
    return nullptr;
}

//<subexpression> := (<expressions>) | (<expressions>)\I
subexpression *parser::parse_subexpression() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::LPAREN) {
        it++;
        auto sub = new subexpression();
        auto id_expressions = parse_expressions();
        if(!id_expressions){
            throw std::invalid_argument("Couldn't resolve expressions");
        }
        lex.set_current_token(it, last);
        if(lex.get_current_token() == lexer::RPAREN){
            it++;
            auto id_ignore = parse_ignore();
            if(id_ignore){
                id_ignore->children.push_back(id_expressions);
                sub->children.push_back(id_ignore);
                return sub;
            }
            sub->children.push_back(id_expressions);
            return sub;
        }
        throw std::invalid_argument("Couldn't resolve subexpression");
    }
    return nullptr;
}

//<expressions> := <expression> | <expression><expressions> | <subexpression> | <subexpression><expressions>
expressions *parser::parse_expressions() {
    auto id_subexpression = parse_subexpression();
    if(id_subexpression){
        auto result = new expressions();
        //it++;
        result->children.push_back(id_subexpression);
        auto new_expression = parse_expressions();
        if(new_expression){
            result->children.push_back(new_expression);
        }
        return result;
    }
    auto id_expression = parse_expression();
    if(!id_expression){
        return nullptr;
    }
    auto result = new expressions();
    result->children.push_back(id_expression);
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::RPAREN){
        return result;
    }
    auto new_expression = parse_expressions();
    if(new_expression){
        result->children.push_back(new_expression);
    }
    return result;
}

//<count> := <string>{<digit>}
count *parser::parse_count() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::LBRACKET){
        auto c = new count();
        it++;
        lex.set_current_token(it, last);
        if(lex.get_current_token() == lexer::DIGIT){
            c->digit = it;
            it++;
            lex.set_current_token(it, last);
            if(lex.get_current_token() == lexer::RBRACKET){
                it++;
                return c;
            }
        }
        throw std::invalid_argument("Couldn't resolve count");
    }
    return nullptr;
}

//<level> := <expressions>\O{<digit>}
level *parser::parse_level() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::LEVEL_OP){
        it = it + 2;
        lex.set_current_token(it, last);
        if(lex.get_current_token() == lexer::LBRACKET){
            it++;
            lex.set_current_token(it, last);
            if(lex.get_current_token() == lexer::DIGIT){
                auto lvl = new level();
                lvl->digit = it;
                it++;
                lex.set_current_token(it, last);
                if(lex.get_current_token() == lexer::RBRACKET){
                    return lvl;
                }
            }
        }
        throw std::invalid_argument("Couldn't resolve level");
    }
    return nullptr;
}

//<ignore> := \I
ignore *parser::parse_ignore() {
    lex.set_current_token(it, last);
    if(lex.get_current_token() == lexer::IGNORE_OP){
        it = it + 2;
        auto i = new ignore();
        return i;
    }
    return nullptr;
}
