#pragma once

#define NUM_SYMBOLS 1024 //max number of symbol table entries

enum {
    S_VARIABLE,
    S_FUNCTION
};

typedef struct symbol_table_entry_t {
	char *name;
    int type; //primitive type
    int structure_type; //variable, function, etc
} symbol_table_entry_t;

typedef struct symbol_table_t {
    symbol_table_entry_t table[NUM_SYMBOLS];
    int next_free; // position of next free slot
} symbol_table_t;

int find_symbol(char *s, symbol_table_t *symbol_table);
int next_symbol_pos(symbol_table_t *symbol_table);
int create_symbol(char *name, int type, int structure_type, symbol_table_t *symbol_table);