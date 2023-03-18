// By: Vincent Lazo, Christian Manuel
// parse given file, checking for valid syntax against given grammar while building an AST

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "ast.h"
#include "utilities.h"

#define STMTBEGINTOKS 7

static token currToken;
static token_type can_begin_stmt[STMTBEGINTOKS] = 
{identsym, beginsym, ifsym, whilesym, readsym, writesym, skipsym};

// go to next token
static void advance()
{
	if (!lexer_done())
		currToken = lexer_next();
}

// check if currToken is the appropriate token type
void eat(token_type tt)
{
	if (currToken.typ == tt)
	{
        advance();
    }
	else 
	{
		token_type expected[1] = {tt};
		parse_error_unexpected(expected, 1, currToken);
    }
}

// call lexer open
void parser_open(const char *filename)
{
	lexer_open(filename);
	currToken = lexer_next();
}

// call lexer close
void parser_close()
{
	lexer_close();
}

// parse program to return AST
// <program> ::= <block> EOF
AST *parseyParse()
{
	AST *block = parseBlock();
	eat(eofsym);

	// might be more stuff to do here

	return block;
}

// parse const & var decls and stmt
// <block> ::= {<const-decls>} {<var-decls>} <stmt>
AST *parseBlock()
{
	AST_list const_defs = parseConstDecls();
	AST_list var_decls = parseVarDecls();
	AST *stmt = parseStmts();

	// gives AST's starting location (7.2.1 in pdf)
	file_location floc;

	// checks if there is an empty const decl or var decl list; if so, set file location to the
	// first non-empty list
	if (!ast_list_is_empty(const_defs) || !ast_list_is_empty(var_decls))
	{
		if (!ast_list_is_empty(const_defs))
		{
			if (ast_list_first(const_defs)->type_tag == const_decl_ast)
			{
				floc = ast_list_first(const_defs)->file_loc;
			}
			else
			{
				bail_with_error("Bad AST for const declarations");
			}
		}
		// must be an else if to act in sequence
		else if (!ast_list_is_empty(var_decls))
		{
			if (ast_list_first(var_decls)->type_tag == var_decl_ast)
			{
				floc = ast_list_first(var_decls)->file_loc;
			}
			else
			{
				bail_with_error("Bad AST for var declarations");
			}
		}
	}
	// if both lists are empty (i.e. there are no const/ var decls) set file loc to stmt
	else if (stmt != NULL)
	{
		floc = stmt->file_loc;
	}
	// (might be unnecessary) if theres no program
	else
	{
		// might have to bail_with_error?
		return NULL;
	}

	return ast_program(floc.filename, floc.line, floc.column, const_defs, var_decls, stmt);
}

// adds newTail onto end of existing AST_list
static void add_AST_to_end(AST_list *head, AST_list *tail, AST_list newTail)
{
	if (ast_list_is_empty(*head))
	{
		*head = newTail;
		*tail = ast_list_last_elem(newTail);
	}
	else
	{
		ast_list_splice(*tail, newTail);
		*tail = ast_list_last_elem(newTail);
	}
}

// -----------------------------const defs-----------------------------

// <const-decls> ::= {<const-decl>}
// i.e. there can be 0 or more const decls
// wrapper function for parseConstDecl
AST_list parseConstDecls()
{
	AST_list ret = ast_list_empty_list();

	// checks if there even exists any const defs
	if (currToken.typ == constsym)
		ret = parseConstDecl();

	return ret;
}

// <const-decl> ::= const <const-def> {<comma-const-def>} ;
// 							   ^			^
// 					<ident> = <number>	  , <ident> = <number>
AST_list parseConstDecl()
{
	AST_list ret, last, new_const_def;

	eat(constsym);

	ret = parseConstDef();
	last = ret;

	while (currToken.typ == commasym)
	{
		eat(commasym);

		// currToken is currently an ident, so append it to ret
		new_const_def = parseConstDef();
		add_AST_to_end(&ret, &last, new_const_def);
	}

	return ret;
}

// parse a const definition
// <const-def> ::= <ident> = <number>
AST_list parseConstDef()
{
	// remember ident token
	token idToken = currToken;
	token numToken;
	
	// move on to =
	eat(identsym);

	// move on to number
	eat(eqsym);

	numToken = currToken;

	eat(numbersym);

	return ast_list_singleton(ast_const_def(idToken, idToken.text, numToken.value));
}

// -----------------------------var decls-----------------------------

// <var-decls> ::= {<var-decl>}
// wrapper function for parseVarDecl
AST_list parseVarDecls()
{
	AST_list ret = ast_list_empty_list();

	// checks if there even exists any var decls
	if (currToken.typ == varsym)
		ret = parseVarDecl();

	return ret;
}

// <var-decl> ::= var <idents>
// <idents> ::= <ident> {<comma-ident>}
// 							  ^
// 						, <ident>
AST_list parseVarDecl()
{
	AST_list ret, last, new_var_decl;

	eat(varsym);

	ret = parseVarIdent();
	last = ret;

	while (currToken.typ == commasym)
	{
		eat(commasym);

		new_var_decl = parseVarIdent();
		add_AST_to_end(&ret, &last, new_var_decl);
	}

	return ret;
}

// go to next ident
AST_list parseVarIdent()
{
	token idToken = currToken;

	eat(identsym);

	return ast_list_singleton(ast_var_decl(idToken, idToken.text));
}

// -----------------------------stmts-----------------------------

// may not need
// wrapper function for parseStmt
AST *parseStmts()
{
	AST *ret = NULL;

	// checks if there even are any stmts
	if (is_stmt_beginning_token(currToken))
		ret = parseStmt();

	return ret;
}

// checks if token can begin a stmt
bool is_stmt_beginning_token(token t)
{
	for (int i = 0; i < STMTBEGINTOKS; i++)
	{
		if (currToken.typ == can_begin_stmt[i])
			return true;
	}

	return false;
}

// <stmt> ::= <ident> := <expr> | ...
AST* parseStmt()
{
	AST *ret = NULL;

	switch (currToken.typ)
	{
		case identsym:
			ret = parseAssignStmt();
			break;
		case beginsym:
			ret = parseBeginStmt();
			break;
		case ifsym:
			ret = parseIfStmt();
			break;
		case whilesym:
			ret = parseWhileStmt();
			break;
		case readsym:
			ret = parseReadStmt();
			break;
		case writesym:
			ret = parseWriteStmt();
			break;
		case skipsym:
			ret = parseSkipStmt();
			break;
		default:
			parse_error_unexpected(can_begin_stmt, STMTBEGINTOKS, currToken);
	}

	return ret;
}

// <ident> := <expr>
AST* parseAssignStmt()
{
	token idToken = currToken;

	eat(identsym);
	eat(becomessym);

	AST *exp = parseExpr();

	return ast_assign_stmt(idToken, idToken.text, exp);
}

// maybe create a 
// <expr> ::= <expr> <bin-arith-op> <expr> | <ident> | <number>
AST* parseExpr()
{
	AST *ret = NULL;
	token tempToken;

	switch (currToken.typ)
	{
		case identsym:
			ret = parseIdent();
			break;
		case numbersym:
			ret = parseNumber();
			break;
		// how to handle the <expr> <bin-arith-op> <expr> case?
		default:
			break;
	}

	return ret;
}

AST *parseIdent()
{
	token idToken = currToken;

	eat(identsym);

	return ast_list_singleton(ast_ident(idToken, idToken.text));
}

AST* parseNumber()
{
	token numToken = currToken;

	eat(numbersym);

	return ast_list_singleton(ast_number(numToken, numToken.value));
}

AST* parseBinExpr()
{

}

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

AST* parseArithOp();
