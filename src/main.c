#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include "parser.h"
#include "lex.h"
#include "codegen.h"

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
	node->ast_type = ast_type;
	node->left = NULL;
	node->right = NULL;
	return node;
}

int get_math_operator(int token) {
	switch(token) {
		case T_PLUS:
			return AST_ADD;
		case T_MINUS:
			return AST_SUBTRACT;
		case T_STAR:
			return AST_MULTIPLY;
		case T_SLASH:
			return AST_DIVIDE;
		case T_PERCENT:
			return AST_MOD;
		default:
			printf("Unknown operator %d\n", token);
			exit(1);
	}
}

//see lex.h for the corresponding tokens (first are + - * / %)
static int operator_precedences[] = {10, 10, 20, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int operator_precedence(int token) {
	int precedence = operator_precedences[token];
	if(precedence == 0) {
		printf("Invalid operator: %d\n", token);
		exit(1);
	}
	return precedence;
}

ast_node_t *create_expression_ast(parser_t *parser, int ptp) {
	if(parser->lexer->tokens[parser->pos]->token != T_INTLIT) {
		printf("Expected expression with integer value on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}
	ast_node_t *left = create_ast_leaf(AST_INTLIT);
	ast_node_t *right;
	left->int_value = parser->lexer->tokens[parser->pos]->int_value;

	parser->pos++; //goes from number to operator

	if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) return left;
	if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
		printf("Unexpected end of file, missing semicolon?\n");
		exit(1);
	}

	int token_type = parser->lexer->tokens[parser->pos]->token;

	while(operator_precedence(parser->lexer->tokens[parser->pos]->token) > ptp) {
		parser->pos++;

		right = create_expression_ast(parser, operator_precedences[token_type]);

		left = create_ast_node(get_math_operator(token_type), left, right);
		left->int_value = parser->lexer->tokens[parser->pos]->int_value;

		if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) break;
		if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
			printf("Unexpected end of file, missing semicolon?\n");
			exit(1);
		}
		token_type = parser->lexer->tokens[parser->pos]->token;
	}

	return left;
}

void parse(parser_t *parser) {
	for(; parser->pos < parser->lexer->token_count; parser->pos++) {
		token_t *current_token = parser->lexer->tokens[parser->pos];
		if(current_token == NULL) return;

		if(current_token->keyword == K_RETURN) {
			ast_node_t *return_node = create_ast_leaf(AST_RETURN);
			parser->root_node->left = return_node;

			parser->pos++;
			return_node->left = create_expression_ast(parser, 0);
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

int alloc_register(code_gen_t *code_gen) {
	for(int i = 0; i < 4; i++) {
		if(code_gen->free_regs[i]) {
			code_gen->free_regs[i] = 0;
			return i;
		}
	}
	printf("No more registers left!\n");
	exit(1);
}

void free_register(int reg, code_gen_t *code_gen) {
	if(code_gen->free_regs[reg] != 0) return;
	code_gen->free_regs[reg] = 1;
}

int cg_load(int value, code_gen_t *code_gen) {
	int reg = alloc_register(code_gen);
	fprintf(code_gen->out, "    mov %s, %d\n", code_gen->reg_list[reg], value);
	return reg;
}

int cg_add(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    add %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	free_register(r2, code_gen);
	return r1;
}

int cg_sub(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    sub %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	free_register(r2, code_gen);
	return r1;
}

int cg_mul(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    mul %s\n", code_gen->reg_list[r1]);
	fprintf(code_gen->out, "    mov %s, rax\n", code_gen->reg_list[r1]);
	free_register(r2, code_gen);
	return r1;
}

int cg_div(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", code_gen->reg_list[r1]);
	fprintf(code_gen->out, "    mov rbx, %s\n", code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    div rbx\n");
	fprintf(code_gen->out, "    mov %s, rax\n", code_gen->reg_list[r2]); //rax = quotient
	free_register(r1, code_gen);
	return r2;
}

int cg_mod(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", code_gen->reg_list[r1]);
	fprintf(code_gen->out, "    mov rbx, %s\n", code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    div rbx\n");
	fprintf(code_gen->out, "    mov %s, rdx\n", code_gen->reg_list[r2]); //rdx = remainder
	free_register(r1, code_gen);
	return r2;
}

int cg_expression(ast_node_t *root_node, code_gen_t *code_gen) {
	int r1;
	int r2;

	if(root_node->left != NULL) r1 = cg_expression(root_node->left, code_gen);
	if(root_node->right != NULL) r2 = cg_expression(root_node->right, code_gen);

	switch(root_node->ast_type) {
		case AST_ADD:
			return cg_add(r1, r2, code_gen);
		case AST_SUBTRACT:
			return cg_sub(r1, r2, code_gen);
		case AST_MULTIPLY:
			return cg_mul(r1, r2, code_gen);
		case AST_DIVIDE:
			return cg_div(r1, r2, code_gen);
		case AST_MOD:
			return cg_mod(r1, r2, code_gen);
		case AST_INTLIT:
			return cg_load(root_node->int_value, code_gen);
		default:
			printf("Unknown operator %d\n", root_node->ast_type);
			exit(1);
	}
}

void generate_return(ast_node_t *return_node, code_gen_t *code_gen) {
	int result_reg = cg_expression(return_node->left, code_gen);
	if(result_reg < 0) exit(1);
	fprintf(code_gen->out, "    mov rdi, %s\n", code_gen->reg_list[result_reg]);
	fprintf(code_gen->out, "    mov rax, 60\n");
	fprintf(code_gen->out, "    syscall\n");
}

void code_generation(code_gen_t *code_gen) {
	ast_node_t *root_node = code_gen->parser->root_node;
	if(root_node->ast_type == AST_PROGRAM) { //should always be
		root_node = root_node->left;
		if(root_node->ast_type == AST_RETURN) {
			generate_return(root_node, code_gen);
		}
	}
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

	fprintf(lexer.out_file, "global main\n");
	fprintf(lexer.out_file, "section .text\n");
	fprintf(lexer.out_file, "  main:\n");

	code_gen_t code_gen;
	code_gen.out = lexer.out_file;
	code_gen.parser = &parser;
	code_gen.reg_list[0] = "r8";
	code_gen.reg_list[1] = "r9";
	code_gen.reg_list[2] = "r10";
	code_gen.reg_list[3] = "r11";
	memset(code_gen.free_regs, 1, 4);

	code_generation(&code_gen);
	//fprintf(lexer.out_file, "	movq $2, %%rax\n");
	//fprintf(lexer.out_file, "	ret\n");

	fclose(lexer.in_file);
	fclose(lexer.out_file);

	return 0;
}

/*
Basic return value in ASM (not valid anymore lol)

.globl main

main:               ; label for "main" function
	movq $2, %rax   ; move constant 2 into rax
	ret             ; return rax
*/