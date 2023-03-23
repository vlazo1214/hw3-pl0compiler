#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include "ast.h"
#include "token.h"

void advance();

void eat(token_type tt);

// call lexer open
void parser_open(const char *filename);

// call lexer close
void parser_close();

// parse program to return AST
AST *parseyParse();

AST *parseBlock();

AST_list parseConstDecls();

AST_list parseConstDecl(token const_sym);

AST_list parseConstIdent(token idTemp);

AST_list parseVarDecls();

AST_list parseVarDecl();

AST_list parseVarIdent(token idTemp);

AST *parseStmts();

AST* parseStmt();

bool is_stmt_beginning_token(token t);

AST* parseAssignStmt();

AST_list parseExpr();

AST *parseTerm();

AST *parseFactor();

AST *parseMultDivFactor();

bool is_a_sign(token_type tt);

AST *parseAddSubTerm();

AST *parseOpExpr(token remember);

AST *parseIdentExpr();

AST* parseNumber();

AST *parseParenExpr();

AST* parseBeginStmt();

AST* parseStmtList();

AST* parseIfStmt();

rel_op which_one();

AST* parseWhileStmt();

AST* parseReadStmt();

AST* parseWriteStmt();

AST* parseSkipStmt();

AST* parseCondition();

AST* parseOddCond();

AST* parseBinRelCond();

AST* parseRelOp();

AST* parseBinExpr();

AST* parseArithOp();

#endif