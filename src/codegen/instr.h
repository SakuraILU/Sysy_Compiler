#pragma once

#include "koopa.h"
#include <iostream>
#include <unordered_map>
#include <string.h>
#include <assert.h>

class InstrHandler
{
private:
    int nreg;

    // std::unordered_map<uintptr_t, int> binstr_outregs;
    std::string regs[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
    int NREG;

    int max_offset;
    std::unordered_map<uintptr_t, int> stk_offsets;

    void get_regs(koopa_raw_binary_t binary, std::string &lreg, std::string &rreg)
    {
        auto &lkind = binary.lhs->kind;
        int nreg_tmp = nreg;
        if (lkind.tag == KOOPA_RVT_INTEGER)
        {
            if (lkind.data.integer.value == 0)
            {
                lreg = "x0";
            }
            else
            {
                std::cout << "  li  " << regs[nreg_tmp] << ", " << lkind.data.integer.value << std::endl;
                lreg = regs[nreg_tmp++];
            }
        }
        else if (lkind.tag == KOOPA_RVT_BINARY || lkind.tag == KOOPA_RVT_LOAD || lkind.tag == KOOPA_RVT_ALLOC)
        {
            std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&lkind] << "(sp)" << std::endl;
            lreg = regs[nreg_tmp++];
        }
        else
            assert(false);

        auto &rkind = binary.rhs->kind;
        if (rkind.tag == KOOPA_RVT_INTEGER)
        {
            std::cerr << "rhs" << std::endl;
            if (rkind.data.integer.value == 0)
            {
                rreg = "x0";
            }
            else
            {
                std::cout << "  li  " << regs[nreg_tmp] << ", " << rkind.data.integer.value << std::endl;
                rreg = regs[nreg_tmp++];
            }
        }
        else if (rkind.tag == KOOPA_RVT_BINARY || rkind.tag == KOOPA_RVT_LOAD || rkind.tag == KOOPA_RVT_ALLOC)
        {
            std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&rkind] << "(sp)" << std::endl;
            rreg = regs[nreg_tmp++];
        }
        else
            assert(false);
    }

    void get_reg(koopa_raw_store_t store, std::string &reg)
    {
        auto &kind = store.value->kind;
        int nreg_tmp = nreg;
        if (kind.tag == KOOPA_RVT_INTEGER)
        {
            if (kind.data.integer.value == 0)
            {
                reg = "x0";
            }
            else
            {
                std::cout << "  li  " << regs[nreg_tmp] << ", " << kind.data.integer.value << std::endl;
                reg = regs[nreg_tmp++];
            }
        }
        else if (kind.tag == KOOPA_RVT_BINARY || kind.tag == KOOPA_RVT_LOAD || kind.tag == KOOPA_RVT_ALLOC)
        {
            std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;
            reg = regs[nreg_tmp++];
        }
        else
            assert(false);
    }

public:
    InstrHandler() : stk_offsets(std::unordered_map<uintptr_t, int>())
    {
        nreg = 0;
        NREG = sizeof(regs) / sizeof(regs[0]);

        max_offset = 0;
    };

    void reset()
    {
        stk_offsets.clear();
        nreg = 0;
        max_offset = 0;
    }

    void ret_handler(const koopa_raw_value_kind_t &kind)
    {
        assert(kind.tag == KOOPA_RVT_RETURN);
        auto &ret = kind.data.ret;
        if (ret.value->kind.tag == KOOPA_RVT_BINARY || ret.value->kind.tag == KOOPA_RVT_LOAD)
        {
            std::cout << "  lw  a0, " << stk_offsets[(uintptr_t)&ret.value->kind] << "(sp)" << std::endl;
        }
        else if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
            std::cout << "  li a0, " << ret.value->kind.data.integer.value << std::endl;
    }

    void binary_handler(const koopa_raw_value_kind_t &kind)
    {
        assert(kind.tag == KOOPA_RVT_BINARY);
        assert(nreg < NREG);

        auto &binary = kind.data.binary;
        // std::cerr << "instr" << std::endl;
        // // std::cout << binary.op << std::endl;
        // std::cerr << binary.lhs->kind.tag << std::endl;
        // std::cerr << binary.rhs->kind.tag << std::endl;

        std::string lreg, rreg;
        get_regs(binary, lreg, rreg);
        switch (binary.op)
        {
        case KOOPA_RBO_NOT_EQ:
        {
            std::cout << "  and  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            break;
        }
        case KOOPA_RBO_EQ:
        {
            std::cout << "  xor  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            break;
        }
        case KOOPA_RBO_SUB:
        {
            std::cout << "  sub  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_ADD:
        {
            std::cout << "  add  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_MUL:
        {
            std::cout << "  mul  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_DIV:
        {
            std::cout << "  div  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_MOD:
        {
            std::cout << "  rem  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_LE:
        {
            std::cout << "  sgt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            break;
        }
        case KOOPA_RBO_GE:
        {
            std::cout << "  slt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            break;
        }
        case KOOPA_RBO_LT:
        {
            std::cout << "  slt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_GT:
        {
            std::cout << "  sgt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_AND:
        {
            std::cout << "  and  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        case KOOPA_RBO_OR:
        {
            std::cout << "  or  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            break;
        }
        default:
        {
            std::cerr << "unspported operation idx " << binary.op << std::endl;
            assert(false);
        }
        }

        stk_offsets[(uintptr_t)&kind] = max_offset;
        max_offset += 4;
        std::cout << "  sw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;
    }

    void store_handler(const koopa_raw_value_kind_t &kind)
    {
        auto &store = kind.data.store;
        std::cerr << "tag is " << store.dest->kind.tag << std::endl;
        if (stk_offsets.count((uintptr_t)&store.dest->kind) == 0)
        {
            stk_offsets[(uintptr_t)&store.dest->kind] = max_offset;
            max_offset += 4;
        }
        std::string reg;
        get_reg(store, reg);
        std::cout << "  sw " << reg << ", " << stk_offsets[(uintptr_t)&kind.data.store.dest->kind] << "(sp)" << std::endl;
    }

    void load_handler(const koopa_raw_value_kind_t &kind)
    {
        auto &load = kind.data.load;

        std::cerr << "tag is " << load.src->kind.tag << std::endl;
        assert(stk_offsets.count((uintptr_t)&load.src->kind.data.store.dest->kind) == 0);
        std::cout << "  lw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&load.src->kind] << "(sp)" << std::endl;

        stk_offsets[(uintptr_t)&kind] = max_offset;
        max_offset += 4;
        std::cout << "  sw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;

        nreg++;
    }
};