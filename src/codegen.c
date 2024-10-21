#include "codegen.h"

#include <string.h>

//longs (8)
static char *reg_list[] = { "r8", "r9", "r10", "r11" };
//ints (4)
static char *dreg_list[] = { "r8d", "r9d", "r10d", "r11d" };
//shorts (2)
static char *wreg_list[] = { "r8w", "r9w", "r10w", "r11w" };
//chars (1)
static char *breg_list[] = { "r8b", "r9b", "r10b", "r11b" };

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

void free_all_registers(code_gen_t *code_gen) {
	memset(code_gen->free_regs, 1, 4);
}

int get_type_size(int type) {
	switch(type) {
		case P_VOID:
		case P_NONE:
			return 0;
		case P_UCHAR:
		case P_CHAR:
			return 1;
		case P_USHORT:
		case P_SHORT:
			return 2;
		case P_UINT:
		case P_INT:
			return 4;
		case P_ULONG:
		case P_LONG:
		case P_VOIDPTR:
		case P_CHARPTR:
		case P_SHORTPTR:
		case P_INTPTR:
		case P_LONGPTR:
			return 8;
		default:
			printf("Could not get type size\n");
			exit(1);
	}
}

//puts an int literal into a register
int cg_load(int value, code_gen_t *code_gen) {
	int reg = alloc_register(code_gen);
	fprintf(code_gen->out, "    mov %s, %d\n", reg_list[reg], value);
	return reg;
}

int cg_add(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    add %s, %s\n", reg_list[r1], reg_list[r2]);
	free_register(r2, code_gen);
	return r1;
}

int cg_sub(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    sub %s, %s\n", reg_list[r1], reg_list[r2]);
	free_register(r2, code_gen);
	return r1;
}

int cg_mul(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", reg_list[r2]);
	fprintf(code_gen->out, "    mul %s\n", reg_list[r1]);
	fprintf(code_gen->out, "    mov %s, rax\n", reg_list[r1]);
	free_register(r2, code_gen);
	return r1;
}

int cg_div(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", reg_list[r1]);
	fprintf(code_gen->out, "    mov rbx, %s\n", reg_list[r2]);
	fprintf(code_gen->out, "    div rbx\n");
	fprintf(code_gen->out, "    mov %s, rax\n", reg_list[r2]); //rax = quotient
	free_register(r1, code_gen);
	return r2;
}

int cg_mod(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    mov rax, %s\n", reg_list[r1]);
	fprintf(code_gen->out, "    mov rbx, %s\n", reg_list[r2]);
	fprintf(code_gen->out, "    div rbx\n");
	fprintf(code_gen->out, "    mov %s, rdx\n", reg_list[r2]); //rdx = remainder
	free_register(r1, code_gen);
	return r2;
}

//puts a value from register into the stack
int cg_assign_variable(int id, int reg, code_gen_t *code_gen) {
	int location;

	int size = get_type_size(get_symbol(id, code_gen->parser->symbol_table)->type);
	if(get_symbol(id, code_gen->parser->symbol_table)->stack_pos == -1) {
		code_gen->stack_pos += size;
	}	

	char *reg_name;
	char *size_name;

	//Adds keeps the memory addresses byte aligned to their respective data type sizes

	if(size == 1) {
		reg_name = breg_list[reg];
		size_name = "BYTE";
		if(get_symbol(id, code_gen->parser->symbol_table)->stack_pos == -1) {
			location = code_gen->stack_pos;
		}
	}
	if(size == 2) {
		reg_name = wreg_list[reg];
		size_name = "WORD";
		if(get_symbol(id, code_gen->parser->symbol_table)->stack_pos == -1) {
			code_gen->stack_pos += code_gen->stack_pos % 2;
			location = code_gen->stack_pos;
		}
	}
	if(size == 4) {
		reg_name = dreg_list[reg];
		size_name = "DWORD";
		if(get_symbol(id, code_gen->parser->symbol_table)->stack_pos == -1) {
			code_gen->stack_pos += 4 - 1 - (code_gen->stack_pos + 4 - 1) % 4;
			location = code_gen->stack_pos;
		}
	}
	if(size == 8) {
		reg_name = reg_list[reg];
		size_name = "QWORD";
		if(get_symbol(id, code_gen->parser->symbol_table)->stack_pos == -1) {
			code_gen->stack_pos += 8 - 1 - (code_gen->stack_pos + 8 - 1) % 8;
			location = code_gen->stack_pos;
		}
	}

	if(get_symbol(id, code_gen->parser->symbol_table)->stack_pos == -1) {
		get_symbol(id, code_gen->parser->symbol_table)->stack_pos = code_gen->stack_pos;
	}else {
		location = get_symbol(id, code_gen->parser->symbol_table)->stack_pos;
	}

	fprintf(code_gen->out, "    mov %s -%d[rbp], %s", size_name, location, reg_name);
	if(code_gen->parser->add_debug_comments) {
		fprintf(code_gen->out, " ; [Assign] ID: %s", get_symbol(id, code_gen->parser->symbol_table)->name);
	}
	fprintf(code_gen->out, "\n");

	return reg;
}

//puts value from stack into register
int cg_retrieve_variable(int id, code_gen_t *code_gen) {
	int reg = alloc_register(code_gen);
	int size = get_type_size(get_symbol(id, code_gen->parser->symbol_table)->type);
	int pos = get_symbol(id, code_gen->parser->symbol_table)->stack_pos;

	char *reg_name;
	char *size_name;

	if(size == 1) {
		reg_name = breg_list[reg];
		size_name = "BYTE";
	}
	if(size == 2) {
		reg_name = wreg_list[reg];
		size_name = "WORD";
	}
	if(size == 4) {
		reg_name = dreg_list[reg];
		size_name = "DWORD";
	}
	if(size == 8) {
		reg_name = reg_list[reg];
		size_name = "QWORD";
	}

	fprintf(code_gen->out, "    mov %s, %s -%d[rbp]", reg_name, size_name, pos);
	if(code_gen->parser->add_debug_comments) {
		fprintf(code_gen->out, " ; [Retrieve] ID: %s", get_symbol(id, code_gen->parser->symbol_table)->name);
	}
	fprintf(code_gen->out, "\n");

	return reg;
}

int cg_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", reg_list[r1], reg_list[r2]);
	fprintf(code_gen->out, "    setz al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_not_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", reg_list[r1], reg_list[r2]);
	fprintf(code_gen->out, "    setne al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_less_than(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", reg_list[r1], reg_list[r2]);
	fprintf(code_gen->out, "    setl al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_less_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", reg_list[r1], reg_list[r2]);
	fprintf(code_gen->out, "    setle al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_greater_than(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", reg_list[r1], reg_list[r2]);
	fprintf(code_gen->out, "    setg al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_greater_equals(int r1, int r2, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    cmp %s, %s\n", reg_list[r1], reg_list[r2]);
	fprintf(code_gen->out, "    setge al\n");
	fprintf(code_gen->out, "    movzx %s, al\n", reg_list[r2]);
	free_register(r1, code_gen);
	return r2;
}

int cg_call(char *name, code_gen_t *code_gen) {
	int reg = alloc_register(code_gen);
	fprintf(code_gen->out, "    call _%s\n", (char *)name);
	fprintf(code_gen->out, "    mov %s, rax\n", reg_list[reg]); //takes the return value of the function
	return reg;
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
		case AST_CALL:
			fprintf(code_gen->out, "    push %s\n", reg_list[r1]); //the function call might override our working register
			int reg = cg_call(root_node->name, code_gen);
			fprintf(code_gen->out, "    pop %s\n", reg_list[r1]);
			return reg;
		case AST_WIDEN:
			return r1;
		default:
			printf("Unknown operator %d\n", root_node->ast_type);
			exit(1);
	}
}

void cg_label(int id, code_gen_t *code_gen) {
	fprintf(code_gen->out, "  L%d:\n", id);
}

void generate_return(ast_node_t *return_node, code_gen_t *code_gen) {
	int result_reg = cg_expression(return_node->left, code_gen);
	if(result_reg < 0) exit(1);
	fprintf(code_gen->out, "    mov rax, %s\n", reg_list[result_reg]);
	fprintf(code_gen->out, "    mov rsp, rbp\n");
	fprintf(code_gen->out, "    pop rbp\n");
	fprintf(code_gen->out, "    ret\n");
}

void generate_assignment(ast_node_t *assignment_node, code_gen_t *code_gen) {
	int symbol_id = assignment_node->right->symbol_id; //right side is lvalue (ie, the var name)
	ast_node_t *expression_node = assignment_node->left;
	int expression_reg = cg_expression(expression_node, code_gen);
	cg_assign_variable(symbol_id, expression_reg, code_gen);
}

void generate_ifelse(ast_node_t *if_node, code_gen_t *code_gen) {
	int condition_reg = cg_expression(if_node->left, code_gen);
	int label_false = code_gen->label_index++;
	int label_end = code_gen->label_index++;
	
	fprintf(code_gen->out, "    cmp %s, 0\n", reg_list[condition_reg]); //compare condition to zero
	fprintf(code_gen->out, "    je L%d\n", label_false);
	code_generation(code_gen, if_node->mid);
	free_all_registers(code_gen);
	fprintf(code_gen->out, "    jmp L%d\n", label_end);
	cg_label(label_false, code_gen);
	code_generation(code_gen, if_node->right);
	fprintf(code_gen->out, "    jmp L%d\n", label_end);
	cg_label(label_end, code_gen);
}

void generate_if(ast_node_t *if_node, code_gen_t *code_gen) {
	if(if_node->right != NULL) { //if an else statement exists
		generate_ifelse(if_node, code_gen);
		return;
	}

	int condition_reg = cg_expression(if_node->left, code_gen);
	int label_end = code_gen->label_index++;
	
	fprintf(code_gen->out, "    cmp %s, 0\n", reg_list[condition_reg]); //compare condition to zero
	fprintf(code_gen->out, "    je L%d\n", label_end);
	code_generation(code_gen, if_node->mid);
	free_all_registers(code_gen);
	fprintf(code_gen->out, "    jmp L%d\n", label_end);
	cg_label(label_end, code_gen);
}

void generate_while(ast_node_t *while_node, code_gen_t *code_gen) {
	int label_start = code_gen->label_index++;
	int label_end = code_gen->label_index++;

	fprintf(code_gen->out, "    jmp L%d\n", label_start);
	cg_label(label_start, code_gen);

	code_generation(code_gen, while_node->right); //the body of the while loop

	int condition_reg = cg_expression(while_node->left, code_gen); //evaluate the expression
	fprintf(code_gen->out, "    cmp %s, 0", reg_list[condition_reg]); //compare condition to zero

	if(code_gen->parser->add_debug_comments) {
		fprintf(code_gen->out, " ; [While/For Cond.]");
	}
	fprintf(code_gen->out, "\n");

	fprintf(code_gen->out, "    jne L%d\n", label_start); //loop back if condition met
	fprintf(code_gen->out, "    je L%d\n", label_end); //exit if condition not met

	cg_label(label_end, code_gen);
}

void generate_function(ast_node_t *function_node, code_gen_t *code_gen) {
	fprintf(code_gen->out, "  _%s:\n", (char *)function_node->name);
	fprintf(code_gen->out, "    push rbp\n");
	fprintf(code_gen->out, "    mov rbp, rsp\n");
	fprintf(code_gen->out, "    sub rsp, 64\n");
	code_generation(code_gen, function_node->left);
}

void generate_asm(ast_node_t *asm_node, code_gen_t *code_gen) {
	fprintf(code_gen->out, "    %s\n", asm_node->asm_line);
}

void code_generation(code_gen_t *code_gen, ast_node_t *node) {
	if(node == NULL) {
		printf("Node has returned null!\n");
		exit(1);
	}
	switch(node->ast_type) {
		case AST_ASM:
			generate_asm(node, code_gen);
			break;
		case AST_RETURN:
			generate_return(node, code_gen);
			break;
		case AST_INT:
			if(node->right == NULL) break;
			generate_assignment(node->right, code_gen);
			free_all_registers(code_gen);
			break;
		case AST_ASSIGN:
			generate_assignment(node, code_gen);
			free_all_registers(code_gen);
			break;
		case AST_BLOCK:
			if(node->left != NULL) code_generation(code_gen, node->left);
			if(node->right != NULL) code_generation(code_gen, node->right);
			free_all_registers(code_gen);
			break;
		case AST_IF:
			generate_if(node, code_gen);
			free_all_registers(code_gen);
			break;
		case AST_WHILE:
			generate_while(node, code_gen);
			free_all_registers(code_gen);
			break;
		case AST_FOR: //Same as block but left in as formality
			if(node->left != NULL) code_generation(code_gen, node->left);
			if(node->right != NULL) code_generation(code_gen, node->right);
			free_all_registers(code_gen);
			break;
		case AST_FUNCTION:
			generate_function(node, code_gen);
			free_all_registers(code_gen);
			break;
		case AST_CALL:
			cg_call(node->name, code_gen);
			free_all_registers(code_gen);
			break;
		default:
			printf("Unknown AST type: %d\n", node->ast_type);
			exit(1);
	}
}