#pragma once

#include "koopa.h"
#include <iostream>
#include <unordered_map>
#include <string.h>
#include <assert.h>
#include "layout.h"

class InstrHandler
{
private:
    int nreg;

    std::string regs[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
    int NREG;

    int max_offset;
    std::unordered_map<uintptr_t, int> stk_offsets;

    int reg_idx(std::string &reg);

    void get_regs(koopa_raw_binary_t binary, std::string &lreg, std::string &rreg);

    void get_reg(koopa_raw_store_t store, std::string &reg);

public:
    InstrHandler();

    void reset();

    void ret_handler(const koopa_raw_value_kind_t &kind);

    void binary_handler(const koopa_raw_value_kind_t &kind);

    void store_handler(const koopa_raw_value_kind_t &kind);

    void load_handler(const koopa_raw_value_kind_t &kind);

    void branch_handler(const koopa_raw_value_kind_t &kind);

    void jump_handler(const koopa_raw_value_kind_t &kind);

    void call_handler(const koopa_raw_value_kind_t &kind);

    void galloc_handler(const koopa_raw_value_t &value);
};
