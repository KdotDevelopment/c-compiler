#pragma once

#include <stdio.h>
#include <stdint.h>

#define MAX_IDENT_LENGTH 32

typedef struct token_t {
	int token;
	int int_value;
	int keyword;
	char ident_value[MAX_IDENT_LENGTH];
	int line_num;
} token_t;

typedef struct lexer_t {
	FILE *in_file;
	FILE *out_file;
	size_t line_num;
	int8_t character;
	size_t token_count;
	size_t token_index;
	token_t **tokens;
} lexer_t;

int lex(lexer_t *lexer);
int clean_tokens(lexer_t *lexer);

//Characters or Combinations
enum {
	T_PLUS,
	T_MINUS,
	T_STAR,
	T_SLASH,
	T_DOUBLE_SLASH,   // //
	T_PERCENT,
	T_EQUALS,         // ==
	T_NOT_EQUALS,     // !=
	T_LESS_THAN,      // <
	T_GREATER_THAN,   // >
	T_LESS_EQUALS,    // <=
	T_GREATER_EQUALS, // >=

	//These do not have any precedences:

	T_NOT,            // ! 11
	T_ASSIGN,         // =
	T_LOGAND,         // &&
	T_LOGOR,          // ||
	T_BITOR,          // |
	T_AMPERSAND,      // &
	T_DOLLAR,         // $ - used for asm instructions
	T_LBRACKET,
	T_RBRACKET,
	T_SINGLE_QUOTE,
	T_DOUBLE_QUOTE,
	T_IDENT,
	T_SEMICOLON,
	T_LPAREN,
	T_RPAREN,
	T_COMMA,
	T_LSQUIRLY,
	T_RSQUIRLY,
	T_INTLIT,
	T_EOF
};

//Keywords
enum {
	K_IDENT, //This is for user-made variable names
	K_ASM, //extension
	K_AUTO,
	K_BREAK,
	K_CASE,
	K_CHAR,
	K_CONST,
	K_CONTINUE,
	K_DEFAULT,
	K_DO,
	K_DOUBLE,
	K_ELSE,
	K_ENUM,
	K_EXTERN,
	K_FLOAT,
	K_FOR,
	K_GOTO,
	K_IF,
	K_INLINE, //C99
	K_INT,
	K_LONG,
	K_REGISTER,
	K_RESTRICT, //C99
	K_RETURN,
	K_SHORT,
	K_SIGNED,
	K_SIZEOF,
	K_STATIC,
	K_STRUCT,
	K_SWITCH,
	K_TYPEDEF,
	K_UNION,
	K_UNSIGNED,
	K_VOID,
	K_VOLATILE,
	K_WHILE
};