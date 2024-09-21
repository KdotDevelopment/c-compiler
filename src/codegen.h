#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "parser.h"

typedef struct code_gen_t {
    uint8_t free_regs[4];
    char *reg_list[4];
    FILE *out;
    size_t label_index;
    parser_t *parser;
} code_gen_t;

void code_generation(code_gen_t *code_gen, ast_node_t *node);