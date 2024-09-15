#pragma once

#include "parser.h"
#include <stdio.h>
#include <stdint.h>

typedef struct code_gen_t {
    uint8_t free_regs[4];
    char *reg_list[4];
    FILE *out;
    parser_t *parser;
} code_gen_t;