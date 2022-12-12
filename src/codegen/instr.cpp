#include "instr.h"

int InstrHandler::reg_idx(std::string &reg)
{
    for (int i = 0; i < NREG; ++i)
    {
        if (regs[i].compare(reg) == 0)
            return i;
    }
    std::cerr << "invalid registers" << std::endl;
    assert(false);
}

void InstrHandler::get_regs(koopa_raw_binary_t binary, std::string &lreg, std::string &rreg)
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
    else if (lkind.tag == KOOPA_RVT_BINARY || lkind.tag == KOOPA_RVT_LOAD || lkind.tag == KOOPA_RVT_ALLOC || lkind.tag == KOOPA_RVT_CALL)
    {
        std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&lkind] << "(sp)" << std::endl;
        lreg = regs[nreg_tmp++];
    }
    else
    {
        std::cerr << "kind " << lkind.tag << " is not supported " << std::endl;
        assert(false);
    }

    auto &rkind = binary.rhs->kind;
    if (rkind.tag == KOOPA_RVT_INTEGER)
    {
        // std::cerr << "rhs" << std::endl;
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
    else if (rkind.tag == KOOPA_RVT_BINARY || rkind.tag == KOOPA_RVT_LOAD || rkind.tag == KOOPA_RVT_ALLOC || rkind.tag == KOOPA_RVT_CALL)
    {
        std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&rkind] << "(sp)" << std::endl;
        rreg = regs[nreg_tmp++];
    }
    else
    {
        std::cerr << "kind " << rkind.tag << " is not supported " << std::endl;
        assert(false);
    }
}

void InstrHandler::get_reg(koopa_raw_store_t store, std::string &reg)
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
    else if (kind.tag == KOOPA_RVT_CALL)
    {
        reg = ARGREG(0);
    }
    else if (kind.tag == KOOPA_RVT_FUNC_ARG_REF)
    {
        int arg_idx = kind.data.func_arg_ref.index;
        if (arg_idx < 8)
            std::cout << "  mv " << regs[nreg_tmp] << ", " << ARGREG(arg_idx) << std::endl;
        else
        {
            std::cerr << STKSIZE + (arg_idx - 8) * 4 << "(sp)" << std::endl;
            std::cout << "  lw " << regs[nreg_tmp] << ", " << 256 + 4 + (arg_idx - 8) * 4 << "(sp)" << std::endl;
        }
        reg = regs[nreg_tmp++];
    }
    else
    {
        std::cerr << "kind " << kind.tag << " is not supported " << std::endl;
        assert(false);
    }
}

InstrHandler::InstrHandler() : stk_offsets(std::unordered_map<uintptr_t, int>())
{
    nreg = 0;
    max_offset = STKOFF;
};

void InstrHandler::reset()
{
    stk_offsets.clear();
    nreg = 0;
    max_offset = STKOFF;
}

void InstrHandler::ret_handler(const koopa_raw_value_kind_t &kind)
{
    // std::cerr << "ret" << std::endl;
    assert(kind.tag == KOOPA_RVT_RETURN);
    auto &ret = kind.data.ret;
    if (ret.value == nullptr)
    {
        std::cout << "  addi sp, sp, " << STKSIZE << std::endl;
        std::cout << "  ret" << std::endl;
        return;
    }

    if (ret.value->kind.tag == KOOPA_RVT_BINARY || ret.value->kind.tag == KOOPA_RVT_LOAD)
    {
        std::cout << "  lw  a0, " << stk_offsets[(uintptr_t)&ret.value->kind] << "(sp)" << std::endl;
        std::cout << "  addi sp, sp, " << STKSIZE << std::endl;
        std::cout << "  ret" << std::endl;
    }
    else if (ret.value->kind.tag == KOOPA_RVT_CALL)
    {
        std::cout << "  addi sp, sp, " << STKSIZE << std::endl;
        std::cout << "  ret" << std::endl;
    }
    else if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
    {
        std::cout << "  li a0, " << ret.value->kind.data.integer.value << std::endl;
        std::cout << "  addi sp, sp, " << STKSIZE << std::endl;
        std::cout << "  ret" << std::endl;
    }
    else
    {
        std::cerr << "kind " << kind.tag << " is not supported " << std::endl;
        assert(false);
    }
}

void InstrHandler::binary_handler(const koopa_raw_value_kind_t &kind)
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
        std::cout << "  xor  " << regs[nreg] << ", " << lreg << ", " << rreg << std::endl;
        std::cout << "  snez  " << regs[nreg] << ", " << regs[nreg] << std::endl;
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

void InstrHandler::store_handler(const koopa_raw_value_kind_t &kind)
{
    assert(nreg < NREG);
    auto &store = kind.data.store;
    // std::cerr << "tag is " << store.dest->kind.tag << std::endl;
    if (stk_offsets.count((uintptr_t)&store.dest->kind) == 0)
    {
        stk_offsets[(uintptr_t)&store.dest->kind] = max_offset;
        max_offset += 4;
    }

    std::string reg;
    get_reg(store, reg);

    if (store.value->kind.tag == KOOPA_RVT_ALLOC || store.value->kind.tag == KOOPA_RVT_BINARY || store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF || store.value->kind.tag == KOOPA_RVT_CALL)
    {
        if (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
        {
            std::cout << "  la " << regs[reg_idx(reg) + 1] << ", " << store.dest->name + 1 << std::endl;
            std::cout << "  sw " << reg << ", (" << regs[reg_idx(reg) + 1] << ")" << std::endl;
        }
        else
            std::cout << "  sw " << reg << ", " << stk_offsets[(uintptr_t)&store.dest->kind] << "(sp)" << std::endl;
    }
    else
    {
        std::cerr << "unsupported tag " << store.value->kind.tag << std::endl;
        assert(false);
    }
}

void InstrHandler::load_handler(const koopa_raw_value_kind_t &kind)
{
    assert(nreg < NREG);
    auto &load = kind.data.load;
    // std::cerr << " load type " << load.src->kind.tag << std::endl;

    if (load.src->kind.tag == KOOPA_RVT_ALLOC || load.src->kind.tag == KOOPA_RVT_BINARY)
    {
        // std::cerr << "tag is " << load.src->kind.tag << std::endl;
        assert(stk_offsets.count((uintptr_t)&load.src->kind) != 0);
        std::cout << "  lw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&load.src->kind] << "(sp)" << std::endl;
    }
    else if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        std::cout << "  la " << regs[nreg] << ", " << load.src->name + 1 << std::endl;
        std::cout << "  lw " << regs[nreg] << ", (" << regs[nreg] << ")" << std::endl;
    }

    stk_offsets[(uintptr_t)&kind] = max_offset;
    max_offset += 4;
    std::cout << "  sw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;
}

void InstrHandler::branch_handler(const koopa_raw_value_kind_t &kind)
{
    assert(kind.tag == KOOPA_RVT_BRANCH);
    auto &branch = kind.data.branch;
    std::cout << "  lw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&branch.cond->kind] << "(sp)" << std::endl;
    std::cout << "  bnez " << regs[nreg] << ", " << branch.true_bb->name + 1 << std::endl;
    std::cout << "  j " << branch.false_bb->name + 1 << std::endl;
}

void InstrHandler::jump_handler(const koopa_raw_value_kind_t &kind)
{
    assert(kind.tag == KOOPA_RVT_JUMP);
    auto &jump = kind.data.jump;
    std::cout << "  j " << jump.target->name + 1 << std::endl;
}

void InstrHandler::call_handler(const koopa_raw_value_kind_t &kind)
{
    assert(kind.tag == KOOPA_RVT_CALL);
    auto &call = kind.data.call;

    std::cout << "  sw ra, " << RASTKOFF << "(sp)" << std::endl;
    int arg_stk_off = ARGSTKOFF;
    for (int i = 0; i < call.args.len; ++i)
    {
        auto &arg_kind = ((koopa_raw_value_t)call.args.buffer[i])->kind;
        if (i < 8)
        {
            std::cout << "  lw " << ARGREG(i) << ", " << stk_offsets[(uintptr_t)&arg_kind] << "(sp)" << std::endl;
        }
        else
        {
            std::cout << "  lw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&arg_kind] << "(sp)" << std::endl;
            std::cout << "  sw " << regs[nreg] << ", " << arg_stk_off << "(sp)" << std::endl;
            arg_stk_off += 4;
        }
    }
    std::cout << "  call " << call.callee->name + 1 << std::endl;

    stk_offsets[(uintptr_t)&kind] = max_offset;
    max_offset += 4;
    std::cout << "  sw a0, " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;

    std::cout << "  lw ra, " << RASTKOFF << "(sp)" << std::endl;
}

void InstrHandler::galloc_handler(const koopa_raw_value_t &value)
{
    assert(value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC);

    std::string ident = value->name + 1;
    auto &init = value->kind.data.global_alloc.init;
    // std::cerr << "global name is " << value->name + 1 << std::endl;
    // std::cerr << "init kind is " << value->kind.data.global_alloc.init->kind.tag << std::endl;
    std::cout << "  .global " << ident << std::endl;
    std::cout << ident << ":" << std::endl;
    if (init->kind.tag == KOOPA_RVT_INTEGER)
        std::cout << "  .word " << init->kind.data.integer.value << std::endl;
    else if (init->kind.tag == KOOPA_RVT_ZERO_INIT)
        std::cout << "  .zero 4" << std::endl;
}
