#include "codegen.h"

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

//puts an int literal into a register
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
	fprintf(code_gen->out, "    mov %s, rax ; rax = quotient from div\n", code_gen->reg_list[r2]); //rax = quotient
	free_register(r1, code_gen);
	return r2;
}

int cg_mod(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", code_gen->reg_list[r1]);
	fprintf(code_gen->out, "    mov rbx, %s\n", code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    div rbx\n");
	fprintf(code_gen->out, "    mov %s, rdx ; rdx = remainder from div\n", code_gen->reg_list[r2]); //rdx = remainder
	free_register(r1, code_gen);
	return r2;
}

//puts a value from register into the stack
int cg_assign_variable(int id, int reg, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov QWORD -%d[rbp], %s\n", (id + 1) * 8, code_gen->reg_list[reg]); // * 8 because it is a QWORD (8 bytes)
	return reg;
}

//puts value from stack into register
int cg_retrieve_variable(int id, code_gen_t *code_gen) {
	int reg = alloc_register(code_gen);
	fprintf(code_gen->out, "    mov QWORD %s, -%d[rbp]\n", code_gen->reg_list[reg], (id + 1) * 8); // * 8 because it is a QWORD (8 bytes)
	return reg;
}

int cg_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    setz al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", code_gen->reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_not_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    setne al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", code_gen->reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_less_than(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    setl al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", code_gen->reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_less_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    setle al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", code_gen->reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_greater_than(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    setg al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", code_gen->reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_greater_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", code_gen->reg_list[r1], code_gen->reg_list[r2]);
	fprintf(code_gen->out, "    setge al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", code_gen->reg_list[r2]);
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
		case AST_EQUALS:
			return cg_equals(r1, r2, code_gen);
		case AST_NOT_EQUALS:
			return cg_not_equals(r1, r2, code_gen);
		case AST_LESS_THAN:
			return cg_less_than(r1, r2, code_gen);
		case AST_LESS_EQUALS:
			return cg_less_equals(r1, r2, code_gen);
		case AST_GREATER_THAN:
			return cg_greater_than(r1, r2, code_gen);
		case AST_GREATER_EQUALS:
			return cg_greater_equals(r1, r2, code_gen);
		case AST_INTLIT:
			return cg_load(root_node->int_value, code_gen);
		case AST_IDENT:
			return cg_retrieve_variable(root_node->symbol_id, code_gen);
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

void generate_assignment(ast_node_t *assignment_node, code_gen_t *code_gen) {
	int symbol_id = assignment_node->right->symbol_id; //right side is lvalue (ie, the var name)
	ast_node_t *expression_node = assignment_node->left;
	int expression_reg = cg_expression(expression_node, code_gen);
	cg_assign_variable(symbol_id, expression_reg, code_gen);
}

void code_generation(code_gen_t *code_gen, ast_node_t *node) {
	if(node == NULL) {
		printf("Node has returned null!\n");
		exit(1);
	}
	switch(node->ast_type) {
		case AST_RETURN:
			generate_return(node, code_gen);
			break;
		case AST_INT:
			if(node->right == NULL) break;
			generate_assignment(node->right, code_gen);
			break;
		case AST_ASSIGN:
			generate_assignment(node, code_gen);
			break;
		case AST_BLOCK:
			code_generation(code_gen, node->left);
			code_generation(code_gen, node->right);
			break;
		default:
			printf("Unknown syntax: %d\n", node->ast_type);
			exit(1);
	}
}