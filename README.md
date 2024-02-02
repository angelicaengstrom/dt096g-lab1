# dt096g-lab1
Program som tillandahåller en implementering av ett likartat "regular expression" språk.

Syntaxen beskrivs enligt BNF där
```
    <match> := <level>
    <level> := <expressions>\O{<digit>}
    <expressions> := <expression> | <expression><expressions> | <greedy> | <greedy><expressions>
    <greedy> := <many><expressions> | <many><expressions>)<expressions> | <many>)<expressions> | <many>) | <many><expressions>)
    <subexpression> := (<expressions>) | (<greedy> | (<expressions><greedy>
    <expression> := <operand> | <either> | <many> | <count>
    <either> := <operand>+<operand> | <operand>+<either>
    <many> := <operand>*
    <count> := <operand>{<digit>}
    <ignore> := <subexpression>\I | <string>\I
    <operand> := <ignore> | <subexpression> | <string>
    <string> := <letter> | <wildcard> | <letter><string> | <wildcard><string>
    <letter> := [A-Za-z ']
    <wildcard> := .
```

Implementeringen består i sin tur av en lexikal analys, syntax analys och run-time system, där syntax analysen implementerats enligt en **recursive-descent parser**

## Lexikal analys
Matchar mönster enligt BNF som kategoriseras i tokens

```c++
    enum token{
        LPAREN, RPAREN, LBRACKET, RBRACKET,
        EITHER_OP, MANY_OP,
        WILDCARD, DIGIT, LETTER,
        LEVEL_OP, IGNORE_OP, END
    };
```

## Recursive-descent parser
Konstruerar parseträd som nyttjar polymorfism. Där grammatiken utvärderas från topp till botten.

```c++
    match* parse_match();
    level* parse_level();
    expressions* parse_expressions();
    expression* parse_expression();
    subexpression* parse_subexpression();
    greedy* parse_greedy(many* m);
    either* parse_either(operand* op);
    many* parse_many(operand* op);
    count* parse_count(operand* op);
    operand* parse_operand();
    ignore* parse_ignore(string* s);
    ignore* parse_ignore(subexpression* sub);
    string* parse_string();
```

##Run-time system
Evaluerar parseträdet utefter om en match hittades eller inte för vardera barn av basklassen
```c++
bool evaluate(std::string::iterator &it, std::string::iterator &last) override;
```
