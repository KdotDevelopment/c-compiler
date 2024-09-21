#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symbol.h"

int find_symbol(char *s, symbol_table_t *symbol_table) {
	for(int i = 0; i < symbol_table->next_free; i++) {
		if(*s == *symbol_table->table[i].name && !strcmp(s, symbol_table->table[i].name)) {
			return i;
		}
	}
	return -1;
}

int next_symbol_pos(symbol_table_t *symbol_table) {
	int p;
	if((p = symbol_table->next_free++) >= NUM_SYMBOLS) {
		printf("Too many symbols!\n");
		exit(1);
	}
	return p;
}

int create_symbol(char *name, int type, int structure_type, symbol_table_t *symbol_table) {
	int symbol;

	//if it already exists
	if((symbol = find_symbol(name, symbol_table) != -1)) {
		printf("Symbol %s already exists!\n", name);
		exit(1);
	}

	symbol = next_symbol_pos(symbol_table);
	symbol_table->table[symbol].name = strdup(name);
	symbol_table->table[symbol].type = type;
	symbol_table->table[symbol].structure_type = structure_type;
	return symbol;
}