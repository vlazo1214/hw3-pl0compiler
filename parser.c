// By: Vincent Lazo, Christian Manuel
// parse given file, checking for valid syntax against given grammar, returning an AST

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

#define DEBUG 0

static token currToken;

token_type can_begin_stmt[STMTBEGINTOKS] =
{identsym, beginsym, ifsym, whilesym, readsym, writesym, skipsym};

// go to next token
void advance()
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
// <program> ::= <block> .
AST *parseyParse()
{
	AST *progAST = parseBlock();
	eat(periodsym);
	// eat(eofsym);

	// might be more stuff to do here

	return progAST;
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
	else
	{
		floc = stmt->file_loc;
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

	token const_sym = currToken, idTemp;

	eat(constsym);

	ret = parseConstDef(const_sym);
	last = ret;

	// might need to eat semisym?

	while (currToken.typ == commasym)
	{
		eat(commasym);

		idTemp = currToken;

		// currToken is currently an ident, so append it to ret
		new_const_def = parseConstDef(idTemp);
		add_AST_to_end(&ret, &last, new_const_def);
		
		eat(semisym);
	}

	return ret;
}

// parse a const definition
// <const-def> ::= <ident> = <number>
AST_list parseConstDef(token idTemp)
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

	return ast_list_singleton(ast_const_def(idTemp, idToken.text, numToken.value));
}

// -----------------------------var decls-----------------------------

// <var-decls> ::= {<var-decl>}
// wrapper function for parseVarDecl
AST_list parseVarDecls()
{
	AST_list ret = ast_list_empty_list();
	AST_list last = ret;
	token var_sym = currToken;
	// printf("in parseVarDecls()\n");
	// if (currToken.typ == varsym)

	// checks if there even exists any var decls
	while (currToken.typ == varsym)
	{
		eat(varsym);
		add_AST_to_end(&ret, &last, ast_list_singleton(parseVarDecl(var_sym)));
		eat(semisym);
	}

	return ret;
}

// <var-decl> ::= var <idents> ;
// <idents> ::= <ident> {<comma-ident>}
// 							  ^
// 						, <ident>
AST_list parseVarDecl(token var_sym)
{
	AST_list ret, last, new_var_decl;
	token idTemp;

	ret = ast_list_singleton(parseVarIdent(var_sym));
	last = ret;

	while (currToken.typ == commasym)
	{
		eat(commasym);

		idTemp = currToken;

		new_var_decl = parseVarIdent(idTemp);
		add_AST_to_end(&ret, &last, new_var_decl);
	}


	return ret;
}

// go to next ident
AST_list parseVarIdent(token idTemp)
{
	token idToken = currToken;

	eat(identsym);

	return ast_list_singleton(ast_var_decl(idTemp, idToken.text));
}

// -----------------------------stmts-----------------------------

// may not need
// wrapper function for parseStmt
AST *parseStmts()
{
	AST *ret = NULL;
		
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

// <stmt> ::= <ident> := <expr>
// | begin <stmt> {<semi-stmt>} end
// | if <condition> then <stmt> else <stmt>
// | while <condition> do <stmt>
// | read <ident>
// | write <expr>
// | skip
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
			break;
	}

	return ret;
}

// -----------------------------assign stmt-----------------------------


// i<dent> := <expr>
AST* parseAssignStmt()
{
	token idToken = currToken;

	eat(identsym);
	eat(becomessym);

	AST *exp = parseExpr();

	return ast_assign_stmt(idToken, idToken.text, exp);
}

// <expr> ::= <term> {<add-sub-term>}
// <add-sub-term> ::= <add-sub> <term>
// <add-sub> ::= <plus> | <minus>
AST_list parseExpr()
{
	if (DEBUG)
		printf("in expr\n");

	AST_list ret = parseTerm();
	AST_list last = ret;
	AST *e1 = ast_list_singleton(ret), *e2 = NULL;
	token plus_or_minus;
	bin_arith_op op;

	while (is_a_sign(currToken.typ))
	{
		// keep parsing terms and append to ret
		if (currToken.typ == plussym)
		{
			plus_or_minus = currToken;
			op = addop;
			eat(plussym);
		}
		else
		{
			plus_or_minus = currToken;
			op = subop;
			eat(minussym);
		}

		e2 = ast_op_expr(plus_or_minus, op, ast_list_first(parseTerm()));

		add_AST_to_end(&ret, &last, ast_list_singleton(
			ast_bin_expr(plus_or_minus, e1, op, e2)));
	}

	return ret;
}

// <term> ::= <factor> {<mult-div-factor>}
// <factor> ::= <ident> | <sign> <number> | (<expr>)
// <mult-div-factor> ::= <mult-div> <factor>
// <mult-div> ::= <mult> | <div>
AST_list parseTerm()
{
	if (DEBUG)
		printf("in term\n");
	AST_list ret = parseFactor();
	AST_list last = ret;

	AST *e1 = ast_list_first(ret), *e2 = NULL;

	token mult_or_div;
	bin_arith_op op;

	while (currToken.typ == multsym || currToken.typ == divsym)
	{
		if (currToken.typ == multsym)
		{
			mult_or_div = currToken;
			op = multop;
			eat(multsym);
		}
		else if (currToken.typ == divsym)
		{
			mult_or_div = currToken;
			op = divop;
			eat(divsym);
		}

		e2 = ast_op_expr(mult_or_div, op, ast_list_first(parseFactor()));

		add_AST_to_end(&ret, &last, ast_list_singleton(
			ast_bin_expr(mult_or_div, e1, op, e2)));

		e1 = last;
	}

	return ret;
}

// <factor> ::= <ident> | <sign> <number> | (<expr>)
// <sign> ::= <plus> | <minus> | <empty>
AST_list parseFactor()
{
	AST_list ret = NULL;
	// token numToken;

	if (DEBUG)
		printf("currToken type is %d\n", currToken.typ);
	if (currToken.typ == identsym)
	{
		ret = ast_list_singleton(parseIdent());
	}
	else if (is_a_sign(currToken.typ))
	{
		ret = ast_list_singleton(parseSign());
	}
	else if (currToken.typ == numbersym)
	{
		// numToken = currToken;
		// eat(numbersym);
		// parseTerm();
		ret = ast_list_singleton(parseNumber());
	}
	else
	{
		if (DEBUG)
			printf("in else check\n");
		ret = parseTerm();
	}

	return ret;
}

// is current token a plus or minus?
bool is_a_sign(token_type tt)
{
	if (tt == plussym || tt == minussym)
		return true;
	else
		return false;
}

// <sign> ::= <plus> | <minus> | <empty>
// also account for <number> that follows
AST *parseSign()
{
	AST *ret = NULL;
	token plus_or_minus;
	bin_arith_op op;

	if (currToken.typ == plussym)
	{
		plus_or_minus = currToken;
		op = addop;
		eat(plussym);
	}
	else
	{
		plus_or_minus = currToken;
		op = subop;
		eat(minussym);
	}

	ret = ast_op_expr(plus_or_minus, op, parseNumber());

	return ret;
}

AST *parseIdent()
{
	token idToken = currToken;

	eat(identsym);

	return ast_ident(idToken, idToken.text);
}

AST *parseNumber()
{
	token numToken = currToken;

	eat(numbersym);

	return ast_number(numToken, numToken.value);
}

// AST* parseBinExpr()
// {
// 	AST *ret = NULL;

// 	return ret;
// }

// -----------------------------begin stmt-----------------------------

// <begin-stmt> ::= begin <stmt> {<semi-stmt>} end
AST *parseBeginStmt()
{
	token begin_sym = currToken;
	AST_list ret;
	AST_list last;
	AST_list stmt_list;

	eat(beginsym);

	ret = ast_list_singleton(parseStmt());
	last = ret;

	stmt_list = parseStmtList();

	add_AST_to_end(&ret, &last, ast_list_first(stmt_list));

	eat(endsym);

	return ast_begin_stmt(begin_sym, ast_list_first(ret));
}

// <semi-stmt> ::= ; <stmt>
AST_list parseStmtList()
{
	AST_list ret = ast_list_empty_list();
	AST_list last = ret;
	AST *tempStmt;

	while (currToken.typ == semisym)
	{
		eat(semisym);

		tempStmt = parseStmt();

		add_AST_to_end(&ret, &last, ast_list_singleton(tempStmt));
	}

	return ret;
}

// -----------------------------if stmt-----------------------------

// <if-stmt> ::= if <condition> then <stmt> else <stmt>
AST* parseIfStmt()
{
	if (DEBUG)
		printf("made it to if\n");
	AST *cond_stmt, *then_stmt, *else_stmt;
	token if_sym = currToken;

	eat(ifsym);
	if (DEBUG)
		printf("gonna parse cond\n\n");

	cond_stmt = parseCondition();


	eat(thensym);

	then_stmt = parseStmt();

	if (DEBUG)
		printf("parsed then\n");

	eat(elsesym);

	else_stmt = parseStmt();

	if (DEBUG)
	{
		printf("currToken type is %d\n", currToken.typ);
		printf("made it out if\n");
	}
	return ast_if_stmt(if_sym, cond_stmt, then_stmt, else_stmt);
}

// <condition> ::= odd <expr> | <expr> <rel-op> <expr>
// <rel-op> ::= = | <> | < | <= | > | >=
AST* parseCondition()
{
	AST *ret = NULL, *e1 = NULL, *e2 = NULL;
	token temp;
	rel_op op;

	if (DEBUG)
		printf("in cond\n");

	if (currToken.typ == oddsym)
	{
		if (DEBUG)
			printf("in odd check\n");
		temp = currToken;

		eat(oddsym);

		ret = ast_odd_cond(temp, parseExpr());
	}
	else
	{
		temp = currToken;

		e1 = parseExpr();
		op = which_one();
		e2 = parseExpr();

		ret = ast_bin_cond(temp, e1, op, e2);
	}

	return ret;
}

rel_op which_one()
{
	rel_op op;

	switch (currToken.typ)
	{
		case eqsym:
			op = eqop;
			eat(eqsym);
			break;
		case neqsym:
			op = neqop;
			eat(neqsym);
			break;
		case lessym:
			op = ltop;
			eat(lessym);
			break;
		case leqsym:
			op = leqop;
			eat(leqsym);
			break;
		case gtrsym:
			op = gtop;
			eat(gtrsym);
			break;
		case geqsym:
			op = geqop;
			eat(geqsym);
			break;
		default:
			break;
	}

	return op;
}

// -----------------------------while stmt-----------------------------

// <while-stmt> ::= while <condition> do <stmt>
AST* parseWhileStmt()
{
	AST *cond_stmt, *do_stmt;
	token while_sym = currToken;

	eat(whilesym);

	cond_stmt = parseCondition();

	eat(dosym);

	do_stmt = parseStmt();

	return ast_while_stmt(while_sym, cond_stmt, do_stmt);
}

// -----------------------------read stmt-----------------------------

// <read-stmt> ::= read <ident>
AST* parseReadStmt()
{
	token read_sym = currToken;
	token idToken;

	eat(readsym);

	idToken = currToken;

	eat(identsym);

	return ast_read_stmt(read_sym, idToken.text);
}

// -----------------------------write stmt-----------------------------

// <write-stmt> ::= write <expr>
AST* parseWriteStmt()
{
	AST *ret;
	token write_sym = currToken;

	eat(writesym);

	ret = parseExpr();

	return ast_write_stmt(write_sym, ret);
}

// -----------------------------skip stmt-----------------------------

AST* parseSkipStmt()
{
	token skip_sym = currToken;

	eat(skipsym);

	return ast_skip_stmt(skip_sym);
}
