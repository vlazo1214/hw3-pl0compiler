# hw3-pl0compiler 
## Build a parser by building an AST and a declaration checker by analayzing those ASTs.

### Parser
The parser will build an AST by getting tokens via `lexer.c`, checking for syntax errors along the way/

### Declaration Checker
The declaration checker will comprised of a symbol checker and a scope checker in order to make sure that no constant is read/ wrote to and
no constant nor variable is declared more than once/ used without a declaration. 
