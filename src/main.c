#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "parser.h"
#include "lex.h"
#include "codegen.h"
#include "symbol.h"

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
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
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

	symbol_table_t symbol_table;
	//memset(symbol_table.table, 0, NUM_SYMBOLS * sizeof(symbol_table_entry_t));
	symbol_table.next_free = 0;
	parser.symbol_table = &symbol_table;
	parser.add_debug_comments = 0;

	if(argc >= 3 && strcmp(argv[2], "-d") == 0) {
		parser.add_debug_comments = 1;
	}

	lexer.out_file = fopen(add_file_ext(get_file_name(argv[1]), ".s"), "w"); // w = writes to a file, clears first

	fprintf(lexer.out_file, "global main\n");
	fprintf(lexer.out_file, "section .text\n");
	fprintf(lexer.out_file, "  main:\n");
	//fprintf(lexer.out_file, "    push rbp\n");
	//fprintf(lexer.out_file, "    mov rbp, rsp\n");
	fprintf(lexer.out_file, "    call _main\n");
	fprintf(lexer.out_file, "    mov rdi, rax\n");
	fprintf(lexer.out_file, "    mov rax, 60\n");
	fprintf(lexer.out_file, "    syscall\n");

	//Create stack

	//Parsing handles code-gen on the go
	parse(&parser);

	clean_tokens(&lexer);

	fclose(lexer.in_file);
	fclose(lexer.out_file);

	return 0;
}

/*
Basic return value in ASM

global main
section .text
  main:             ; label for "main" function
	mov rdi, 2      ; move contant 2 into rdi
	mov rax, 60     ; set up syscall #60
	syscall         ; return rdi
*/