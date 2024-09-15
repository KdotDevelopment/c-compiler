#include "parser.h"

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