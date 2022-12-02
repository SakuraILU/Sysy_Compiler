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

    std::unordered_map<uintptr_t, int> binstr_outregs;
    std::string regs[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
    int NREG;

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
                std::cout << "  li  t" << nreg_tmp << ", " << binary.lhs->kind.data.integer.value << std::endl;
                lreg = regs[nreg_tmp];
                ++nreg_tmp;
            }
        }
        else if (lkind.tag == KOOPA_RVT_BINARY)
        {
            lreg = regs[binstr_outregs[(uintptr_t)&lkind]];
        }
        else
            assert(false);

        auto &rkind = binary.rhs->kind;
        if (rkind.tag == KOOPA_RVT_INTEGER)
        {
            if (rkind.data.integer.value == 0)
            {
                rreg = "x0";
            }
            else
            {
                std::cout << "  mv  t" << nreg_tmp << ", " << rkind.data.integer.value << std::endl;
                rreg = regs[nreg_tmp];
                ++nreg_tmp;
            }
        }
        else if (rkind.tag == KOOPA_RVT_BINARY)
        {
            rreg = regs[binstr_outregs[(uintptr_t)&rkind]];
        }
        else
            assert(false);
    }

public:
    InstrHandler() : binstr_outregs(std::unordered_map<uintptr_t, int>())
    {
        nreg = 0;
        NREG = sizeof(regs) / sizeof(regs[0]);
    };

    void ret_handler(const koopa_raw_value_kind_t &kind)
    {
        assert(kind.tag == KOOPA_RVT_RETURN);

        std::cout << "  mv  a0, " << regs[binstr_outregs[(uintptr_t)&kind.data.ret.value->kind]] << std::endl;
        std::cout << "  ret" << std::endl;
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
            ++nreg;
            break;
        }
        case KOOPA_RBO_EQ:
        {
            std::cout << "  xor  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_SUB:
        {
            std::cout << "  sub  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_ADD:
        {
            std::cout << "  add  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_MUL:
        {
            std::cout << "  mul  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_DIV:
        {
            std::cout << "  div  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_MOD:
        {
            std::cout << "  rem  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_LE:
        {
            std::cout << "  sgt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_GE:
        {
            std::cout << "  slt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            std::cout << "  seqz  " << regs[nreg] << ", " << regs[nreg] << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_LT:
        {
            std::cout << "  slt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_GT:
        {
            std::cout << "  sgt  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_AND:
        {
            std::cout << "  and  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        case KOOPA_RBO_OR:
        {
            std::cout << "  or  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
            ++nreg;
            break;
        }
        default:
        {
            std::cerr << "unspported operation idx " << binary.op << std::endl;
            assert(false);
        }
        }

        binstr_outregs[(uintptr_t)&kind] = nreg - 1;

        // std::cerr << "store " << (uintptr_t)&kind << ", " << nreg - 1 << std::endl;
    }
};