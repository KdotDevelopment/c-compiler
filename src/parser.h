#pragma once

#include <stdlib.h>
#include "lex.h"
#include "symbol.h"

typedef struct ast_node_t {
	int ast_type;
	char *name;
	struct ast_node_t *left;
	struct ast_node_t *right;
	struct ast_node_t *mid;
	union {
		int int_value;
		int symbol_id;
	};
} ast_node_t;

typedef struct parser_t {
	lexer_t *lexer;
	size_t pos; //which token we are on
	ast_node_t *root_node;
	symbol_table_t *symbol_table;
} parser_t;

enum {
	AST_PROGRAM = 1,
	AST_ADD,
	AST_SUBTRACT,
	AST_MULTIPLY,
	AST_DIVIDE,
	AST_MOD,
	AST_INTLIT,
	AST_RETURN,
	AST_INT,
	AST_ASSIGN,
	AST_LVALUE,
	AST_IDENT,
	AST_EQUALS,
	AST_NOT_EQUALS,
	AST_GREATER_THAN,
	AST_GREATER_EQUALS,
	AST_LESS_THAN,
	AST_LESS_EQUALS,
	AST_IF,
	AST_ELSE,
	AST_BLOCK
};

static char *ast_type_strings[] = {
	"NULL",
	"PROGRAM",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"NUM",
	"RETURN",
	"INT",
	"ASSIGN",
	"LVALUE",
	"IDENT",
	"EQUALS",
	"NOT_EQUALS",
	"GREATER THAN",
	"GREATER_EQUALS",
	"LESS THAN",
	"LESS EQUALS",
	"IF",
	"ELSE",
	"BLOCK"
};

ast_node_t *create_ast_node(int ast_type, ast_node_t *left, ast_node_t *right);
ast_node_t *create_ast_leaf(int ast_type);

void parse(parser_t *parser);
void print_tree(ast_node_t *root_node, int depth);
ast_node_t *create_expression_ast(parser_t *parser, int ptp);
ast_node_t *if_statement(parser_t *parser, token_t *current_token);
ast_node_t *statement_block(parser_t *parser, token_t *current_token);