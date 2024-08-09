#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include "parser.h"
#include "lex.h"

char buffer[24];
char *get_file_name(char *full_name) {
	int i = 0;
	while(*full_name != 0 && *full_name != '.') {
		buffer[i] = *full_name;
		i++;
		full_name++;
	}

	return buffer;
}

char *add_file_ext(char *base_name, char *ext) {
	char *new_file_name = malloc(sizeof(base_name) + strlen(ext));
	int name_length = strlen(base_name);
	strcpy(new_file_name, base_name);
	strncat(new_file_name, ext, sizeof(ext));
}

ast_node_t *create_ast_node(int ast_type, ast_node_t *left, ast_node_t *right) {
	ast_node_t *node = (ast_node_t *)malloc(sizeof(ast_node_t));
	node->ast_type = ast_type;
	node->left = left;
	node->right = right;
	return node;
}

ast_node_t *create_ast_leaf(int ast_type) {
	ast_node_t *node = (ast_node_t *)malloc(sizeof(ast_node_t));
	node->left = NULL;
	node->right = NULL;
	node->ast_type = ast_type;
	return node;
}

int get_math_operator(token_t *token) {
	switch(token->token) {
		case T_PLUS:
			return AST_ADD;
		case T_MINUS:
			return AST_SUBTRACT;
		case T_STAR:
			return AST_MULTIPLY;
		case T_SLASH:
			return AST_DIVIDE;
		default:
			printf("Unknown operator on line %d\n", token->line_num);
			exit(1);
	}
}

ast_node_t *create_expression_ast(parser_t *parser) {
	if(parser->lexer->tokens[parser->pos]->token != T_INTLIT) {
		printf("Expected expression with integer value on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}
	ast_node_t *left = create_ast_leaf(AST_INTLIT);
	left->int_value = parser->lexer->tokens[parser->pos]->int_value;

	parser->pos++;
	if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) return left;
	if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
		printf("Unexpected end of file, missing semicolon?\n");
		exit(1);
	}
	int math_operator = get_math_operator(parser->lexer->tokens[parser->pos]);

	parser->pos++;

	ast_node_t *right = create_expression_ast(parser);

	return create_ast_node(math_operator, left, right);
}

void parse(parser_t *parser) {
	for(; parser->pos < parser->lexer->token_count; parser->pos++) {
		token_t *current_token = parser->lexer->tokens[parser->pos];
		if(current_token == NULL) return;

		if(current_token->keyword == K_RETURN) {
			ast_node_t *return_node = create_ast_leaf(AST_RETURN);
			parser->root_node->left = return_node;

			parser->pos++;
			return_node->left = create_expression_ast(parser);
			if(return_node->left == NULL) {
				printf("Expected expression after token return on line %d\n", current_token->line_num);
			}
		}
	}
}

char *ast_type_to_string(int ast_type) {
	return ast_type_strings[ast_type];
}

void print_tree(ast_node_t *root_node, int depth) {
	if(root_node == NULL) return;
	for(int i = 0; i < depth - 1; i++) printf("   ");
	if(depth > 0) printf("└──");
	printf("%s - %d\n", ast_type_to_string(root_node->ast_type), root_node->int_value);
	print_tree(root_node->left, depth + 1);
	print_tree(root_node->right, depth + 1);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: rcc <filename>\n");
		return 1;
	}

	lexer_t lexer;

	if((lexer.in_file = fopen(argv[1], "r")) == NULL) {
		printf("Unable to open file: %s\n", argv[1]);
		return 1;
	}

	lex(&lexer);

	parser_t parser;
	parser.lexer = &lexer;
	parser.pos = 0;
	parser.root_node = create_ast_leaf(AST_PROGRAM);

	parse(&parser);

	print_tree(parser.root_node, 0);

	clean_tokens(&lexer);

	lexer.out_file = fopen(add_file_ext(get_file_name(argv[1]), ".s"), "w"); // w = writes to a file, clears first

	fprintf(lexer.out_file, ".globl main\n");
	fprintf(lexer.out_file, "main:\n");
	fprintf(lexer.out_file, "	movq $2, %%rax\n");
	fprintf(lexer.out_file, "	ret\n");

	fclose(lexer.in_file);
	fclose(lexer.out_file);

	return 0;
}

/*
Basic return value in ASM

.globl main

main:               ; label for "main" function
	movq $2, %rax   ; move constant 2 into rax
	ret             ; return rax
*/