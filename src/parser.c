#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "symbol.h"
#include "codegen.h"

ast_node_t *create_ast_node(int ast_type, ast_node_t *left, ast_node_t *right) {
	ast_node_t *node = (ast_node_t *)malloc(sizeof(ast_node_t));
	node->ast_type = ast_type;
	node->left = left;
	node->right = right;
	node->name = NULL;
	node->int_value = -1;
	node->symbol_id = -1;
	node->mid = NULL;
	return node;
}

ast_node_t *create_ast_leaf(int ast_type) {
	ast_node_t *node = (ast_node_t *)malloc(sizeof(ast_node_t));
	node->ast_type = ast_type;
	node->mid = NULL;
	node->left = NULL;
	node->right = NULL;
	node->name = NULL;
	node->int_value = -1;
	node->symbol_id = -1;
	return node;
}

int get_math_operator(int token, int line_num) {
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
		case T_EQUALS:
			return AST_EQUALS;
		case T_NOT_EQUALS:
			return AST_NOT_EQUALS;
		case T_LESS_THAN:
			return AST_LESS_THAN;
		case T_LESS_EQUALS:
			return AST_LESS_EQUALS;
		case T_GREATER_THAN:
			return AST_GREATER_THAN;
		case T_GREATER_EQUALS:
			return AST_GREATER_EQUALS;
		default:
			printf("Unknown operator %d on line %d\n", token, line_num);
			exit(1);
	}
}

//see lex.h for the corresponding tokens (first are + - * / % == != < > <= >=)
static int operator_precedences[] = {10, 10, 20, 20, 20, 30, 30, 40, 40, 40, 40};

int operator_precedence(int token) {
	int precedence = operator_precedences[token];
	if(!precedence) {
		printf("Invalid operator: %d\n", token);
		exit(1);
	}
	return precedence;
}

ast_node_t *create_expression_ast(parser_t *parser, int ptp) {
	if(parser->lexer->tokens[parser->pos]->token != T_INTLIT && parser->lexer->tokens[parser->pos]->token != T_IDENT && parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected expression with integer or variable value on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	ast_node_t *left;
	ast_node_t *right;

	if(parser->lexer->tokens[parser->pos]->token == T_LPAREN) {
		parser->pos++;
		left = create_expression_ast(parser, 0); //when paranthesis happen, reset precedence
		if(parser->lexer->tokens[parser->pos]->token != T_RPAREN) {
			printf("Expecting closing parenthesis on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
			exit(1);
		}
	}

	if(parser->lexer->tokens[parser->pos]->token == T_INTLIT) {
		left = create_ast_leaf(AST_INTLIT);
		left->int_value = parser->lexer->tokens[parser->pos]->int_value;
	}

	if(parser->lexer->tokens[parser->pos]->token == T_IDENT) {
		left = create_ast_leaf(AST_IDENT);
		left->symbol_id = find_symbol(parser->lexer->tokens[parser->pos]->ident_value, parser->symbol_table);
	}

	parser->pos++; //goes from number to operator

	if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) return left;
	if(parser->lexer->tokens[parser->pos]->token == T_RPAREN) return left;
	if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
		printf("Unexpected end of file, missing semicolon?\n");
		exit(1);
	}

	int token_type = parser->lexer->tokens[parser->pos]->token;

	while(operator_precedence(parser->lexer->tokens[parser->pos]->token) > ptp) {
		parser->pos++;
		if(parser->lexer->tokens[parser->pos]->token == T_RPAREN) return left;

		if(parser->lexer->tokens[parser->pos]->token == T_LPAREN) {
			parser->pos++;
			right = create_expression_ast(parser, 0); //when paranthesis happen, reset precedence
			if(parser->lexer->tokens[parser->pos]->token != T_RPAREN) {
				printf("Expecting closing parenthesis on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
				exit(1);
			}
			parser->pos++;
		}else {
			right = create_expression_ast(parser, operator_precedence(token_type));
		}

		left = create_ast_node(get_math_operator(token_type, parser->lexer->tokens[parser->pos]->line_num), left, right);
		left->int_value = parser->lexer->tokens[parser->pos]->int_value;

		if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) break;
		if(parser->lexer->tokens[parser->pos]->token == T_RPAREN) break;
		if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
			printf("Unexpected end of file, missing semicolon?\n");
			exit(1);
		}
		token_type = parser->lexer->tokens[parser->pos]->token;
	}

	return left;
}

ast_node_t *return_statement(parser_t *parser, token_t *current_token) {
    ast_node_t *return_node = create_ast_leaf(AST_RETURN);
    parser->root_node->left = return_node;

    parser->pos++;
    return_node->left = create_expression_ast(parser, 0);
    if(return_node->left == NULL) {
        printf("Expected expression after token return on line %d\n", current_token->line_num);
    }

	return return_node;
}

ast_node_t *assignment_statement(parser_t *parser, token_t *current_token) {
	ast_node_t *assignment_node = create_ast_leaf(AST_ASSIGN);
	parser->root_node->left = assignment_node;
	ast_node_t *left;
	ast_node_t *right;

	int symbol_id;
	if((symbol_id = find_symbol(parser->lexer->tokens[parser->pos]->ident_value, parser->symbol_table)) == -1) {
		printf("Undefined variable %s on line %d\n", parser->lexer->tokens[parser->pos]->ident_value, parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	right = create_ast_leaf(AST_LVALUE);
	right->symbol_id = symbol_id;
	right->name = parser->lexer->tokens[parser->pos]->ident_value;

	parser->pos++;

	if(parser->lexer->tokens[parser->pos]->token != T_ASSIGN) {
		printf("Expecting assignment operator after identifier on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++;

	left = create_expression_ast(parser, 0);

	assignment_node->left = left;
	assignment_node->right = right;
	return assignment_node;
}

ast_node_t *int_declaration(parser_t *parser, token_t *current_token) {
    ast_node_t *variable_node = create_ast_leaf(AST_INT);
    parser->root_node->left = variable_node;

    parser->pos++; //from int to indent

	if(parser->lexer->tokens[parser->pos]->token != T_IDENT) {
		printf("Expected indentifier after token int on line %d\n", current_token->line_num);
		exit(1);
	}
	variable_node->symbol_id = create_symbol(parser->lexer->tokens[parser->pos]->ident_value, parser->symbol_table);

	parser->pos++; //from ident to assign or semi

	if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) return variable_node;
	else if(parser->lexer->tokens[parser->pos]->token == T_ASSIGN) {
		parser->pos--; //from assign to ident
		variable_node->right = assignment_statement(parser, parser->lexer->tokens[parser->pos]);
		//printf("Assignment on same line is not supported yet! (Line: %d)\n", parser->lexer->tokens[parser->pos]->line_num);
	}else if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
		printf("Unexpected end of file, missing semicolon?\n");
		exit(1);
	}else {
		printf("Expected either semicolon or assignment after variable declaration on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

    //variable_node->left = create_expression_ast(parser, 0);
	return variable_node;
}

ast_node_t *statement_block(parser_t *parser, token_t *current_token) {
	//ast_node_t *block_node = create_ast_leaf(AST_BLOCK);
	ast_node_t *left = NULL;
	ast_node_t *current_node;

	parser->pos++; // { -> stmt

	for(; parser->pos < parser->lexer->token_count; parser->pos++) {
		token_t *current_token = parser->lexer->tokens[parser->pos];
		if(current_token == NULL) return NULL;
		if(current_token->token == T_RSQUIRLY) {
			if(left == NULL) return NULL;
			parser->pos++;
			return left;
		}
		switch(current_token->keyword) {
			case K_RETURN:
				current_node = return_statement(parser, current_token);
				break;
			case K_INT:
				current_node = int_declaration(parser, current_token);
				break;
			case K_IDENT:
				current_node = assignment_statement(parser, current_token);
				break;
			case K_IF:
				current_node = if_statement(parser, current_token);
				break;
			default:
				printf("Unknown keyword\n");
				exit(1);
		}
		if(current_node) {
			if(left == NULL) {
				left = current_node;
			}else{
				left = create_ast_node(AST_BLOCK, left, current_node);
			}
		}
	}
}

ast_node_t *if_statement(parser_t *parser, token_t *current_token) {
	ast_node_t *if_node = create_ast_leaf(AST_IF);
    parser->root_node->left = if_node;

	parser->pos++; //if to (hopefully) '('

	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected \'(\' after if statement on line %d", current_token->line_num);
		exit(1);
	}

	parser->pos++; // ( to condition

	if_node->left = create_expression_ast(parser, 0);

	if_node->mid = statement_block(parser, current_token);
}

//currently just a test
ast_node_t *function(parser_t *parser, token_t *current_token) {
	parser->pos++; //void -> {
	
	if(parser->lexer->tokens[parser->pos]->token != T_LSQUIRLY) {
		printf("Expected \'{\' after void declaration on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	return statement_block(parser, parser->lexer->tokens[parser->pos]);
}

void parse(parser_t *parser) {
	for(; parser->pos < parser->lexer->token_count; parser->pos++) {
		token_t *current_token = parser->lexer->tokens[parser->pos];
		ast_node_t *current_node = NULL;
		if(current_token == NULL) return;
		switch(current_token->keyword) {
			case K_VOID:
				current_node = function(parser, current_token);
				break;
			default:
				printf("Not expecting execution code outside of a body\n");
				exit(1);
		}
		if(current_node == NULL) continue;
		print_tree(current_node, 0);

		code_gen_t code_gen;
		code_gen.out = parser->lexer->out_file;
		code_gen.parser = parser;
		code_gen.reg_list[0] = "r8";
		code_gen.reg_list[1] = "r9";
		code_gen.reg_list[2] = "r10";
		code_gen.reg_list[3] = "r11";
		memset(code_gen.free_regs, 1, 4);

		code_generation(&code_gen, current_node);
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