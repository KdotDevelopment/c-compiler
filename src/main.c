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