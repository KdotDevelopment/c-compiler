#pragma once

#define NUM_SYMBOLS 1024 //max number of symbol table entries

typedef struct symbol_table_entry_t {
	char *name;
} symbol_table_entry_t;

typedef struct symbol_table_t {
    symbol_table_entry_t table[NUM_SYMBOLS];
    int next_free; // position of next free slot
} symbol_table_t;

int find_symbol(char *s, symbol_table_t *symbol_table);
int next_symbol_pos(symbol_table_t *symbol_table);
int create_symbol(char *name, symbol_table_t *symbol_table);