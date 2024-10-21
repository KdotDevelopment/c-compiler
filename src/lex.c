#include "lex.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int chr_pos(char *s, int c) {
	char *p;

	p = strchr(s, c);
	return (p ? p - s : -1);
}

int get_next_char(lexer_t *lexer) {
	int c;
	
	c = fgetc(lexer->in_file);
	if(c == '\n') lexer->line_num++;
	return c;
}

int skip_char(lexer_t *lexer) {
	int c;

	c = get_next_char(lexer);
	while(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') c = get_next_char(lexer); // skips over stuff that doesn't matter
	//ungetc(c, lexer->in_file);

	return c;
}

int64_t scan_int(lexer_t *lexer) {
	int k, val = 0;

	while((k = chr_pos("0123456789", lexer->character)) >= 0) {
		val = val * 10 + k;
		lexer->character = get_next_char(lexer);
	}

	ungetc(lexer->character, lexer->in_file);
	return val;
}

int scan_ident(lexer_t *lexer, token_t *token) {
	size_t index = 1;

	token->ident_value[0] = lexer->character; //we already know the first one is an ident

	while(isalpha(lexer->character) || isdigit(lexer->character)) {
		if(index > MAX_IDENT_LENGTH - 1) {
			token->ident_value[index] = 0;
			printf("Indentifier %s on line %ld is over %d characters\n", token->ident_value, lexer->line_num, MAX_IDENT_LENGTH);
			exit(1);
		}
		//printf("%c", lexer->character);
		lexer->character = get_next_char(lexer);
		token->ident_value[index] = (char)lexer->character;
		index++;
	}
	token->ident_value[index - 1] = 0;

	ungetc(lexer->character, lexer->in_file);
	return 1;
}

void get_keyword(token_t *token) { //takes the ident string and converts it to a keyword if applicable
	if(token->token != T_IDENT) return;
	if(!strcmp(token->ident_value, "auto")) token->keyword = K_AUTO;
	else if(!strcmp(token->ident_value, "break")) token->keyword = K_BREAK;
	else if(!strcmp(token->ident_value, "case")) token->keyword = K_CASE;
	else if(!strcmp(token->ident_value, "char")) token->keyword = K_CHAR;
	else if(!strcmp(token->ident_value, "const")) token->keyword = K_CONST;
	else if(!strcmp(token->ident_value, "continue")) token->keyword = K_CONTINUE;
	else if(!strcmp(token->ident_value, "default")) token->keyword = K_DEFAULT;
	else if(!strcmp(token->ident_value, "do")) token->keyword = K_DO;
	else if(!strcmp(token->ident_value, "double")) token->keyword = K_DOUBLE;
	else if(!strcmp(token->ident_value, "else")) token->keyword = K_ELSE;
	else if(!strcmp(token->ident_value, "enum")) token->keyword = K_ENUM;
	else if(!strcmp(token->ident_value, "extern")) token->keyword = K_EXTERN;
	else if(!strcmp(token->ident_value, "float")) token->keyword = K_FLOAT;
	else if(!strcmp(token->ident_value, "for")) token->keyword = K_FOR;
	else if(!strcmp(token->ident_value, "goto")) token->keyword = K_GOTO;
	else if(!strcmp(token->ident_value, "if")) token->keyword = K_IF;
	else if(!strcmp(token->ident_value, "int")) token->keyword = K_INT;
	else if(!strcmp(token->ident_value, "long")) token->keyword = K_LONG;
	else if(!strcmp(token->ident_value, "register")) token->keyword = K_REGISTER;
	else if(!strcmp(token->ident_value, "return")) token->keyword = K_RETURN;
	else if(!strcmp(token->ident_value, "short")) token->keyword = K_SHORT;
	else if(!strcmp(token->ident_value, "signed")) token->keyword = K_SIGNED;
	else if(!strcmp(token->ident_value, "sizeof")) token->keyword = K_SIZEOF;
	else if(!strcmp(token->ident_value, "static")) token->keyword = K_STATIC;
	else if(!strcmp(token->ident_value, "struct")) token->keyword = K_STRUCT;
	else if(!strcmp(token->ident_value, "switch")) token->keyword = K_SWITCH;
	else if(!strcmp(token->ident_value, "typedef")) token->keyword = K_TYPEDEF;
	else if(!strcmp(token->ident_value, "union")) token->keyword = K_UNION;
	else if(!strcmp(token->ident_value, "unsigned")) token->keyword = K_UNSIGNED;
	else if(!strcmp(token->ident_value, "void")) token->keyword = K_VOID;
	else if(!strcmp(token->ident_value, "volatile")) token->keyword = K_VOLATILE;
	else if(!strcmp(token->ident_value, "while")) token->keyword = K_WHILE;

	// NOT OFFICIAL
	/*else if(!strcmp(token->ident_value, "u0")) token->keyword = K_VOID;
	else if(!strcmp(token->ident_value, "u8")) token->keyword = K_CHAR;
	else if(!strcmp(token->ident_value, "u16")) token->keyword = K_SHORT;
	else if(!strcmp(token->ident_value, "u32")) token->keyword = K_INT;
	else if(!strcmp(token->ident_value, "u64")) token->keyword = K_LONG;
	else if(!strcmp(token->ident_value, "i8")) token->keyword = K_CHAR;
	else if(!strcmp(token->ident_value, "i16")) token->keyword = K_SHORT;
	else if(!strcmp(token->ident_value, "i32")) token->keyword = K_INT;
	else if(!strcmp(token->ident_value, "i64")) token->keyword = K_LONG;*/

	else token->keyword = K_IDENT; //Must be a user defined variable name
}

//takes rest of line and puts it into ident
token_t scan_line(lexer_t *lexer) {
	token_t token;
	memset(token.ident_value, 0, MAX_IDENT_LENGTH);
	token.int_value = -1;
	token.token = T_IDENT;
	token.keyword = K_ASM;
	
	int index = 0;

	while(index < 32) {
		lexer->character = get_next_char(lexer);
		if(lexer->character == ';') break;
		token.ident_value[index] = lexer->character;
		index++;
	}

	return token;
}

//skips past everything until newline
void line_comment(lexer_t *lexer) {
	lexer->character = get_next_char(lexer);
	int line_num = lexer->line_num;

	while(lexer->line_num == line_num) {
		lexer->character = get_next_char(lexer);
	}
}

token_t scan(lexer_t *lexer) {
	lexer->character = skip_char(lexer); //this will also go to the next character automagically
	token_t token;
	memset(token.ident_value, 0, MAX_IDENT_LENGTH);
	token.int_value = -1;
	token.keyword = -1;
	char next_char;
	switch(lexer->character) {
		case -1:
			token.token = T_EOF;
			break;
		case '+':
			token.token = T_PLUS;
			break;
		case '-':
			token.token = T_MINUS;
			break;
		case '*':
			token.token = T_STAR;
			break;
		case '%':
			token.token = T_PERCENT;
			break;
		case '/':
			next_char = skip_char(lexer);
			if(next_char == '/') {
				//token.token = T_DOUBLE_SLASH;
				line_comment(lexer);
				token.token = -1;
				return token;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_SLASH;
			break;
		case '=':
			next_char = skip_char(lexer);
			if(next_char == '=') {
				token.token = T_EQUALS;
				break;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_ASSIGN;
			break;
		case ';':
			token.token = T_SEMICOLON;
			break;
		case '(':
			token.token = T_LPAREN;
			break;
		case ')':
			token.token = T_RPAREN;
			break;
		case ',':
			token.token = T_COMMA;
			break;
		case '{':
			token.token = T_LSQUIRLY;
			break;
		case '}':
			token.token = T_RSQUIRLY;
			break;
		case '$':
			token = scan_line(lexer);
			break;
		case '[':
			token.token = T_LBRACKET;
			break;
		case ']':
			token.token = T_RBRACKET;
			break;
		case '!':
			next_char = skip_char(lexer);
			if(next_char == '=') {
				token.token = T_NOT_EQUALS;
				break;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_NOT;
			break;
		case '>':
			next_char = skip_char(lexer);
			if(next_char == '=') {
				token.token = T_GREATER_EQUALS;
				break;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_GREATER_THAN;
			break;
		case '<':
			next_char = skip_char(lexer);
			if(next_char == '=') {
				token.token = T_LESS_EQUALS;
				break;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_LESS_THAN;
			break;
		case '&':
			next_char = skip_char(lexer);
			if(next_char == '&') {
				token.token = T_LOGAND;
				break;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_AMPERSAND;
			break;
		case '|':
			next_char = skip_char(lexer);
			if(next_char == '|') {
				token.token = T_LOGOR;
				break;
			}
			ungetc(next_char, lexer->in_file);
			token.token = T_BITOR;
			break;
		default:
			if(isdigit(lexer->character)) {
				token.int_value = scan_int(lexer);
				token.token = T_INTLIT;
				break;
			}
			if(isalpha(lexer->character)) {
				scan_ident(lexer, &token);
				token.token = T_IDENT;
				break;
			}

			printf("Unrecognized character \'%c\' on line %ld\n", lexer->character, lexer->line_num);
			exit(1);
	}
	if(token.keyword == -1) get_keyword(&token);
	token.line_num = lexer->line_num;
	return token; //successful
}

int clean_tokens(lexer_t *lexer) {
	for(int i = 0; i < lexer->token_count; i++) {
		free(lexer->tokens[i]);
	}
}

int lex(lexer_t *lexer) {
	size_t max_token_count = 128;
	lexer->line_num = 1;
	lexer->tokens = (token_t **)malloc(max_token_count * sizeof(token_t *));

	token_t current_token;
	lexer->token_index = 0;
	while(current_token.token != T_EOF) {
		if(lexer->token_index > max_token_count) {
			max_token_count *= 2;
			lexer->tokens = (token_t **)realloc(lexer->tokens, max_token_count * sizeof(token_t *));
		}
		lexer->token_count = max_token_count;

		token_t *permenant_token = malloc(sizeof(token_t));

		current_token = scan(lexer);
		if(current_token.token == -1) continue;
		strcpy(permenant_token->ident_value, current_token.ident_value);
		permenant_token->int_value = current_token.int_value;
		permenant_token->keyword = current_token.keyword;
		permenant_token->token = current_token.token;
		permenant_token->line_num = current_token.line_num;

		lexer->tokens[lexer->token_index++] = permenant_token;
		//printf("%d - %d . %d - %s \n", current_token.token, current_token.keyword, current_token.int_value, current_token.ident_value);
	}
}