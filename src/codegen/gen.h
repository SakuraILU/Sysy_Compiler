#pragma once

#include "koopa.h"
#include "instr.h"

class CodeGen
{
private:
    InstrHandler instr_handler;

public:
    CodeGen();

    void generate(const char *koopa_str);

private:
    void traverse(const koopa_raw_slice_t &slice);

    void handle(const koopa_raw_function_t &func);

    void handle(const koopa_raw_basic_block_t &bb);

    void handle(const koopa_raw_value_t &value);
};
