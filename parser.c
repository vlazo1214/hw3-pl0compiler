#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

#include "lexer.h"
#include "token.h"
#include "ast.h"

token currToken;

void eat()
{
	currToken = lexer_next();
}

// call lexer open
void parser_open()
{
	lexer_open();
	// initialize any globals (if any)
}

// call lexer close
void parser_close()
{
	lexer_close();
	// set globals to NULL
}

// parse program to return AST
AST *parsey_parse(AST *ast);

void parseBlock(AST *ast, int level);

void parseConstDecls(AST_list cds, int level);

void parseConstDecl(AST *cd, int level);

void parseVarDecls(AST_list vds, int level);

void parseVarDecl(AST *vd, int level);

void newlineAndOptionalSemi(bool addSemiToEnd);

void parseStmt(AST *stmt, int indentLevel, bool addSemiToEnd);

void parseAssignStmt(AST *stmt, int level, bool addSemiToEnd);

void parseBeginStmt(AST *stmt, int level, bool addSemiToEnd);

void parseStmtList(AST *stmt, int level, AST_list rest, bool addSemiToEnd);

void parseIfStmt(AST *stmt, int level, bool addSemiToEnd);

void parseWhileStmt(AST *stmt, int level, bool addSemiToEnd);

void parseReadStmt(AST *stmt, int level, bool addSemiToEnd);

void parseWriteStmt(AST *stmt, int level, bool addSemiToEnd);

void parseSkipStmt(int level, bool addSemiToEnd);

void parseCondition(AST *cond);

void parseOddCond(AST *cond);

void parseBinRelCond(AST *cond);

void parseRelOp(rel_op relop);

void parseExpr(AST *exp);

void parseBinExpr(AST *exp);

void parseArithOp(bin_arith_op op);

void parseIdent(AST *id);

void parseNumber(AST *num);