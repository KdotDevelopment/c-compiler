#pragma once

#include "lex.h"

typedef struct ast_node_t {
	int ast_type;
	int int_value;
	struct ast_node_t *left;
	struct ast_node_t *right;
} ast_node_t;

typedef struct parser_t {
	lexer_t *lexer;
	size_t pos; //which token we are on
	ast_node_t *root_node;
} parser_t;

enum {
	AST_PROGRAM = 1,
	AST_ADD,
	AST_SUBTRACT,
	AST_MULTIPLY,
	AST_DIVIDE,
	AST_INTLIT,
	AST_RETURN
};

char *ast_type_strings[8] = {
	"NULL",
	"PROGRAM",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"NUM",
	"RETURN"
};

ast_node_t *create_ast_node(int ast_type, ast_node_t *left, ast_node_t *right);
ast_node_t *create_ast_leaf(int ast_type);