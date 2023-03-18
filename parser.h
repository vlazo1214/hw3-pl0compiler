#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include "ast.h"
#include "token.h"

void eat();

// call lexer open
void parser_open(const char *filename);

// call lexer close
void parser_close();

// parse program to return AST
AST *parseyParse();

AST *parseBlock();

AST_list parseConstDecls();

AST_list parseConstDecl();

AST_list parseConstDef();

AST_list parseVarDecls();

AST_list parseVarDecl();

AST_list parseVarIdent();

AST *parseIdent();

AST *parseStmts();

AST* parseStmt();

bool is_stmt_beginning_token(token t);

AST* parseAssignStmt();

AST* parseBeginStmt();

AST* parseStmtList();

AST* parseIfStmt();

AST* parseWhileStmt();

AST* parseReadStmt();

AST* parseWriteStmt();

AST* parseSkipStmt();

AST* parseCondition();

AST* parseOddCond();

AST* parseBinRelCond();

AST* parseRelOp();

AST* parseExpr();

AST* parseBinExpr();

AST* parseArithOp();

AST* parseNumber();

#endif