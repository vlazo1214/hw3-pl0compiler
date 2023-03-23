// By: Vincent Lazo, Christian Manuel
// main file, calls parser and declaration checking functions

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "unparser.h"


int main(int argc, char *argv[])
{
	// open input file (lexer_open)/ initialize parser
	parser_open(argv[1]);

	// parse program, return ptr to AST (progAST) 
	AST *progAST = parseyParse();

	// close input file (lexer_close)
	parser_close();

	// unparse program with arguments from stdout and progAST
	unparseProgram(stdout, progAST);

	// initialize symbol table

	// using progAST, build symbol table and check for dupe decls/ undecl'd idents

	return EXIT_SUCCESS;
}
