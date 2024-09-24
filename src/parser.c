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
	node->type = -1;
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
	node->type = -1;
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

int get_type(token_t *token) {
	switch(token->keyword) {
		case K_INT:
			return P_INT;
		case K_CHAR:
			return P_CHAR;
		case K_SHORT:
			return P_SHORT;
		case K_VOID:
			return P_VOID;
		case K_LONG:
			return P_LONG;
		default:
			printf("Unknown type %d on line %d\n", token->keyword, token->line_num);
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

//only_right means only widen the left side to match the right
int type_compatible(int *left, int *right, int only_right) {
	//voids never compatible
	if ((*left == P_VOID) || (*right == P_VOID)) return 0;

	//if theyre the same, theyre compatible
	if (*left == *right) { 
		*left = 0; 
		*right = 0;
		return 1;
	}

	if ((*left == P_CHAR) && (*right == P_INT || *right == P_SHORT)) {
		*left = AST_WIDEN;
		*right = 0;
		return 1;
	}

	if ((*left == P_INT || *left == P_SHORT) && (*right == P_CHAR)) {
		if (only_right) return 0;
		*left = 0;
		*right = AST_WIDEN;
		return 1;
	}

	if((*left == P_SHORT) && (*right == P_INT)) {
		*left = AST_WIDEN;
		*right = 0;
		return 1;
	}

	if((*left == P_INT) && (*right == P_SHORT)) {
		if (only_right) return 0;
		*left = 0;
		*right = AST_WIDEN;
		return 1;
	}

	*left = 0; 
	*right = 0;
	return 1;
}

ast_node_t *create_expression_ast(parser_t *parser, int ptp) {
	if(parser->lexer->tokens[parser->pos]->token != T_INTLIT && parser->lexer->tokens[parser->pos]->token != T_IDENT && parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected expression with integer or variable value on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	ast_node_t *left;
	ast_node_t *right;
	int left_type;
	int right_type;

	if(parser->lexer->tokens[parser->pos]->token == T_LPAREN) {
		parser->pos++;
		left = create_expression_ast(parser, 0); //when paranthesis happen, reset precedence
		if(parser->lexer->tokens[parser->pos]->token != T_RPAREN) {
			printf("Expecting closing parenthesis on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
			exit(1);
		}
	}

	//This favors unsigned over signed and smallest over largest
	if(parser->lexer->tokens[parser->pos]->token == T_INTLIT) {
		left = create_ast_leaf(AST_INTLIT);
		left->int_value = parser->lexer->tokens[parser->pos]->int_value;
		
		//Longs and unsigned not supported yet
		//if(left->int_value >= -9223372036854775807 && left->int_value < 9223372036854775807) left->type = P_LONG;
		//if(left->int_value >= -2147483648 && left->int_value < 2147483648) left->type = P_SHORT;
		//if(left->int_value >= -32768 && left->int_value < 32768) left->type = P_SHORT;
		//if(left->int_value >= -128 && left->int_value < 128) left->type = P_CHAR;
		if(left->int_value >= 0 && left->int_value < 256) left->type = P_CHAR;
		if(left->int_value >= 256 && left->int_value < 65536) left->type = P_SHORT;
		if(left->int_value >= 65536 && left->int_value < 4294967296) left->type = P_INT;
		//if(left->int_value >= 4294967296 && left->int_value <= 18446744073709551615) left->type = P_ULONG;
	}

	if(parser->lexer->tokens[parser->pos]->token == T_IDENT) {
		if(parser->lexer->tokens[parser->pos + 1]->token == T_LPAREN) { //function
			if(parser->lexer->tokens[parser->pos + 2]->token == T_RPAREN) {
				left = create_ast_leaf(AST_CALL);
				left->symbol_id = find_symbol(parser->lexer->tokens[parser->pos]->ident_value, parser->symbol_table);
				left->name = parser->lexer->tokens[parser->pos]->ident_value;
				if(left->symbol_id == -1) {
					printf("Could not find symbol %s on line %d\n", parser->lexer->tokens[parser->pos]->ident_value, parser->lexer->tokens[parser->pos]->line_num);
					exit(1);
				}
				left->type = parser->symbol_table->table[left->symbol_id].type;
				parser->pos += 2;
			}
		}else {
			left = create_ast_leaf(AST_IDENT);
			left->symbol_id = find_symbol(parser->lexer->tokens[parser->pos]->ident_value, parser->symbol_table);
			if(left->symbol_id == -1) {
				printf("Could not find symbol %s on line %d\n", parser->lexer->tokens[parser->pos]->ident_value, parser->lexer->tokens[parser->pos]->line_num);
				exit(1);
			}
			left->type = parser->symbol_table->table[left->symbol_id].type;
		}
	}

	parser->pos++; //goes from number to operator (unless a function)

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

		left_type = left->type;
		right_type = right->type;

		if(!type_compatible(&left_type, &right_type, 1)) {
			printf("Incompatible types on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
			exit(1);
		}

		if(left_type == AST_WIDEN) {
			left = create_ast_node(AST_WIDEN, left, NULL);
			left->type = right->type;
			left_type = left->type;
		}
		if(right_type == AST_WIDEN) {
			right = create_ast_node(AST_WIDEN, right, NULL);
			right->type = left->type;
		}

		left = create_ast_node(get_math_operator(token_type, parser->lexer->tokens[parser->pos]->line_num), left, right);
		left->int_value = parser->lexer->tokens[parser->pos]->int_value;
		left->type = left_type;

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

ast_node_t *return_statement(parser_t *parser) {
    ast_node_t *return_node = create_ast_leaf(AST_RETURN);
    parser->root_node->left = return_node;

    parser->pos++;
    return_node->left = create_expression_ast(parser, 0);
    if(return_node->left == NULL) {
        printf("Expected expression after token return on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
    }

	return return_node;
}

ast_node_t *assignment_statement(parser_t *parser) {
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
	right->type = get_symbol(symbol_id, parser->symbol_table)->type;

	parser->pos++; //ident to =

	if(parser->lexer->tokens[parser->pos]->token != T_ASSIGN) {
		printf("Expecting assignment operator after identifier on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; // = to expr

	left = create_expression_ast(parser, 0);

	int left_type = left->type; //left is the expression
	int right_type = right->type; //right is the lvalue

	if(!type_compatible(&left_type, &right_type, 1)) {
		printf("Incompatible types on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	if(left_type == AST_WIDEN) {
		left = create_ast_node(AST_WIDEN, left, NULL);
		left->type = right_type;
	}

	assignment_node->left = left;
	assignment_node->right = right;
	return assignment_node;
}

ast_node_t *var_declaration(parser_t *parser) {
    ast_node_t *variable_node = create_ast_leaf(AST_INT);
    parser->root_node->left = variable_node;

	int variable_type = get_type(parser->lexer->tokens[parser->pos]);

    parser->pos++; //from type to indent

	if(parser->lexer->tokens[parser->pos]->token != T_IDENT) {
		printf("Expected indentifier after token int on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}
	variable_node->symbol_id = create_symbol(parser->lexer->tokens[parser->pos]->ident_value, variable_type, S_VARIABLE, parser->symbol_table);

	parser->pos++; //from ident to assign or semi

	if(parser->lexer->tokens[parser->pos]->token == T_SEMICOLON) return variable_node;
	else if(parser->lexer->tokens[parser->pos]->token == T_ASSIGN) {
		parser->pos--; //from assign to ident
		variable_node->right = assignment_statement(parser);
	}else if(parser->lexer->tokens[parser->pos]->token == T_EOF) {
		printf("Unexpected end of file, missing semicolon?\n");
		exit(1);
	}else {
		printf("Expected either semicolon or assignment after variable declaration on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	return variable_node;
}

ast_node_t *statement_block(parser_t *parser) {
	//ast_node_t *block_node = create_ast_leaf(AST_BLOCK);
	ast_node_t *left = NULL;
	ast_node_t *current_node;

	parser->pos++; // { -> stmt

	for(; parser->pos < parser->lexer->token_count; parser->pos++) {
		token_t *current_token = parser->lexer->tokens[parser->pos];
		
		if(current_token == NULL) return NULL;
		if(current_token->token == T_RSQUIRLY) {
			if(left == NULL) return NULL;
			return left;
		}

		current_node = statement(parser);

		if(current_node) {
			if(left == NULL) {
				left = current_node;
			}else{
				left = create_ast_node(AST_BLOCK, left, current_node);
			}
		}
	}
}

ast_node_t *while_statement(parser_t *parser) {
	ast_node_t *while_node = create_ast_leaf(AST_WHILE);
    parser->root_node->left = while_node;

	parser->pos++; //if to (hopefully) '('

	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected \'(\' after while statement on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; // ( to condition

	while_node->left = create_expression_ast(parser, 0);

	parser->pos++; // ) to {

	while_node->right = statement(parser); //statement to loop
	
	return while_node;
}

ast_node_t *for_statement(parser_t *parser) {
	ast_node_t *for_node = create_ast_leaf(AST_FOR);
    parser->root_node->left = for_node;

	parser->pos++; //if to (hopefully) '('

	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected \'(\' after for statement on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; // ( to condition

	if(parser->lexer->tokens[parser->pos]->token != T_SEMICOLON) { //sometimes you leave some parts empty
		for_node->left = statement(parser); //pre-operation
	}else {
		for_node->left = NULL;
	}
	
	if(parser->lexer->tokens[parser->pos]->token != T_SEMICOLON) {
		printf("Expected semicolon in for loop on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; //skip semicolon

	for_node->right = create_ast_leaf(AST_WHILE);
	for_node->right->left = create_expression_ast(parser, 0); //the expression (middle part) of the for loop

	if(parser->lexer->tokens[parser->pos]->token != T_SEMICOLON) {
		printf("Expected semicolon in for loop on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; //skip semicolon

	ast_node_t *post_operation = NULL;

	if(parser->lexer->tokens[parser->pos]->token != T_RPAREN) { //sometimes you leave some parts empty
		post_operation = statement(parser); //last statement in the for loop
	}

	parser->pos++; // ) -> compound statement

	for_node->right->right = create_ast_leaf(AST_BLOCK);
	for_node->right->right->left = statement(parser); //the block

	for_node->right->right->right = post_operation;

	return for_node;
}

ast_node_t *function_call(parser_t *parser) {
	ast_node_t *call_node = create_ast_leaf(AST_CALL);
	char *name = parser->lexer->tokens[parser->pos]->ident_value;
	parser->pos++; //name -> (

	//arguments would go here

	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected \'(\' after function call on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; //( -> )

	if(parser->lexer->tokens[parser->pos]->token != T_RPAREN) {
		printf("Expected \')\' after function call on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	if(find_symbol(name, parser->symbol_table) == -1) {
		printf("Function with name %s does not exist\n", name);
		exit(1);
	}

	call_node->name = name;

	parser->pos++;

	return call_node;
}

//Used to figure out if this is a function call or an assignment (test() vs test = ...)
ast_node_t *handle_identifier(parser_t *parser) {
	parser->pos++; // ident -> ( or =
	if(parser->lexer->tokens[parser->pos]->token == T_ASSIGN) {
		parser->pos--; //undo last step
		return assignment_statement(parser);
	}
	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected parenthesis for function call after identifier on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}
	parser->pos--;
	
	return function_call(parser);
}

ast_node_t *statement(parser_t *parser) {
	if(parser->lexer->tokens[parser->pos]->token == T_LSQUIRLY) return statement_block(parser);

	token_t *current_token = parser->lexer->tokens[parser->pos];
	ast_node_t *current_node;
		
	if(current_token == NULL) return NULL;
	
	switch(current_token->keyword) {
		case K_RETURN:
			current_node = return_statement(parser);
			break;
		case K_CHAR:
		case K_SHORT:
		case K_INT:
		case K_LONG:
			current_node = var_declaration(parser);
			break;
		case K_IDENT:
			current_node = handle_identifier(parser);
			break;
		case K_IF:
			current_node = if_statement(parser);
			break;
		case K_WHILE:
			current_node = while_statement(parser);
			break;
		case K_FOR:
			current_node = for_statement(parser);
			break;
		default:
			printf("Unknown keyword %d on line %d\n", current_token->keyword, current_token->line_num);
			exit(1);
	}
	return current_node;
}

ast_node_t *if_statement(parser_t *parser) {
	ast_node_t *if_node = create_ast_leaf(AST_IF);
    parser->root_node->left = if_node;

	parser->pos++; //if to (hopefully) '('

	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected \'(\' after if statement on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; // ( to condition

	if_node->left = create_expression_ast(parser, 0);

	parser->pos++; // ) to {

	if_node->mid = statement(parser);
	parser->pos++;
	
	int last_pos = parser->pos - 1; //don't consume next }

	if(parser->lexer->tokens[parser->pos]->keyword == K_ELSE) {
		parser->pos++;
		if_node->right = statement(parser);
	}else {
		parser->pos = last_pos;
		if_node->right = NULL;
	}
	return if_node;
}

//currently just a test
ast_node_t *function(parser_t *parser) {
	ast_node_t *function_node = create_ast_leaf(AST_FUNCTION);

	int type = get_type(parser->lexer->tokens[parser->pos]);

	parser->pos++; //type -> name

	char *name = parser->lexer->tokens[parser->pos]->ident_value;
	create_symbol(name, type, S_FUNCTION, parser->symbol_table);

	if(parser->lexer->tokens[parser->pos]->keyword != K_IDENT) {
		printf("Expected identifier after void declaration on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	function_node->name = parser->lexer->tokens[parser->pos]->ident_value;

	parser->pos++; //name -> (

	//arguments would go here

	if(parser->lexer->tokens[parser->pos]->token != T_LPAREN) {
		printf("Expected \'(\' after void identifier on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; //( -> )

	if(parser->lexer->tokens[parser->pos]->token != T_RPAREN) {
		printf("Expected \')\' after void identifier on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	parser->pos++; //) -> {
	
	if(parser->lexer->tokens[parser->pos]->token != T_LSQUIRLY) {
		printf("Expected \'{\' after void declaration on line %d\n", parser->lexer->tokens[parser->pos]->line_num);
		exit(1);
	}

	function_node->left = statement(parser);

	return function_node;
}

void parse(parser_t *parser) {
	for(; parser->pos < parser->lexer->token_count; parser->pos++) {
		token_t *current_token = parser->lexer->tokens[parser->pos];
		ast_node_t *current_node = NULL;
		if(current_token == NULL) return;
		if(current_token->token == T_EOF) return;
		switch(current_token->keyword) {
			case K_VOID:
			case K_INT:
			case K_CHAR:
				current_node = function(parser);
				break;
			default:
				printf("Not expecting execution code outside of a body (%d)\n", current_token->token);
				exit(1);
		}
		if(current_node == NULL) continue;

		print_tree(current_node, 0);

		code_gen_t code_gen;
		code_gen.out = parser->lexer->out_file;
		code_gen.parser = parser;
		code_gen.label_index = 0;
		code_gen.reg_list[0] = "r8";
		code_gen.reg_list[1] = "r9";
		code_gen.reg_list[2] = "r10";
		code_gen.reg_list[3] = "r11";
		memset(code_gen.free_regs, 1, 4);
		code_gen.stack_pos = 0;

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
	printf("%s - %d - %s\n", ast_type_to_string(root_node->ast_type), root_node->int_value, root_node->name);
	print_tree(root_node->left, depth + 1);
	print_tree(root_node->mid, depth + 1);
	print_tree(root_node->right, depth + 1);
}