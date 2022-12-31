#include "instr.h"

int InstrHandler::reg_idx(std::string &reg)
{
    for (int i = 0; i < NREG; ++i)
    {
        if (regs[i].compare(reg) == 0)
            return i;
    }
    std::cerr << "invalid registers:" << reg << std::endl;
    assert(false);
}

void InstrHandler::get_regs(koopa_raw_binary_t binary, std::string &lreg, std::string &rreg)
{
    int nreg_tmp = nreg;

    auto &lkind = binary.lhs->kind;
    switch (lkind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        if (lkind.data.integer.value == 0)
            lreg = "x0";
        else
        {
            std::cout << "  li  " << regs[nreg_tmp] << ", " << lkind.data.integer.value << std::endl;
            lreg = regs[nreg_tmp++];
        }
        break;
    }
    case KOOPA_RVT_BINARY:
    case KOOPA_RVT_LOAD:
    case KOOPA_RVT_ALLOC:
    case KOOPA_RVT_CALL:
    {
        std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&lkind] << "(sp)" << std::endl;
        lreg = regs[nreg_tmp++];
        break;
    }
    default:
    {
        std::cerr << "kind " << lkind.tag << " is not supported " << std::endl;
        assert(false);
        break;
    }
    }

    auto &rkind = binary.rhs->kind;
    switch (rkind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        if (rkind.data.integer.value == 0)
            rreg = "x0";
        else
        {
            std::cout << "  li  " << regs[nreg_tmp] << ", " << rkind.data.integer.value << std::endl;
            rreg = regs[nreg_tmp++];
        }
        break;
    }
    case KOOPA_RVT_BINARY:
    case KOOPA_RVT_LOAD:
    case KOOPA_RVT_ALLOC:
    case KOOPA_RVT_CALL:
    {
        std::cout << "  lw " << regs[nreg_tmp] << ", " << stk_offsets[(uintptr_t)&rkind] << "(sp)" << std::endl;
        rreg = regs[nreg_tmp++];
        break;
    }
    default:
    {
        std::cerr << "kind " << rkind.tag << " is not supported " << std::endl;
        assert(false);
        break;
    }
    }
}

void InstrHandler::get_reg(koopa_raw_store_t store, std::string &reg)
{
    auto &kind = store.value->kind;

    switch (kind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        if (kind.data.integer.value == 0)
            reg = "x0";
        else
        {
            std::cout << "  li  " << regs[nreg] << ", " << kind.data.integer.value << std::endl;
            reg = regs[nreg];
        }
        break;
    }
    case KOOPA_RVT_BINARY:
    case KOOPA_RVT_LOAD:
    case KOOPA_RVT_ALLOC:
    {
        std::cout << "  lw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;
        reg = regs[nreg];
        break;
    }
    case KOOPA_RVT_CALL:
    {
        reg = ARGREG(0);
        break;
    }
    case KOOPA_RVT_FUNC_ARG_REF:
    {
        int arg_idx = kind.data.func_arg_ref.index;
        if (arg_idx < 8)
            std::cout << "  mv " << regs[nreg] << ", " << ARGREG(arg_idx) << std::endl;
        else
        {
            std::cout << "  li " << regs[nreg + 1] << ", " << STKSIZE + 4 + (arg_idx - 8) * 4 << std::endl;
            std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
            std::cout << "  lw " << regs[nreg] << ", (" << regs[nreg + 1] << ")" << std::endl;
        }
        reg = regs[nreg];
        break;
    }
    default:
    {
        std::cerr << "store kind " << kind.tag << " is not supported " << std::endl;
        assert(false);
        break;
    }
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
        std::cout << "  li t0, " << STKSIZE << std::endl;
        std::cout << "  add sp, sp, t0" << std::endl;
        std::cout << "  ret" << std::endl;
        return;
    }

    switch (ret.value->kind.tag)
    {
    case KOOPA_RVT_BINARY:
    case KOOPA_RVT_LOAD:
    {
        std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&ret.value->kind] << std::endl;
        std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
        std::cout << "  lw a0, (" << regs[nreg + 1] << ")" << std::endl;
        std::cout << "  li t0, " << STKSIZE << std::endl;
        std::cout << "  add sp, sp, t0" << std::endl;
        std::cout << "  ret" << std::endl;
        break;
    }
    case KOOPA_RVT_CALL:
    {
        std::cout << "  li t0, " << STKSIZE << std::endl;
        std::cout << "  add sp, sp, t0" << std::endl;
        std::cout << "  ret" << std::endl;
        break;
    }
    case KOOPA_RVT_INTEGER:
    {
        std::cout << "  li a0, " << ret.value->kind.data.integer.value << std::endl;
        std::cout << "  li t0, " << STKSIZE << std::endl;
        std::cout << "  add sp, sp, t0" << std::endl;
        std::cout << "  ret" << std::endl;
        break;
    }
    default:
    {
        std::cerr << "kind " << kind.tag << " is not supported " << std::endl;
        assert(false);
        break;
    }
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
    // if (inReljumpRange(stk_offsets[(uintptr_t)&kind]))
    //     std::cout << "  sw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&kind] << "(sp)" << std::endl;
    // else
    // {
    std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&kind] << std::endl;
    std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
    std::cout << "  sw " << regs[nreg] << ", (" << regs[nreg + 1] << ")" << std::endl;
    // }
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

    switch (store.dest->kind.tag)
    {
    case KOOPA_RVT_GLOBAL_ALLOC:
    {
        std::cout << "  la " << regs[reg_idx(reg) + 1] << ", " << store.dest->name + 1 << std::endl;
        std::cout << "  sw " << reg << ", (" << regs[reg_idx(reg) + 1] << ")" << std::endl;
        break;
    }
    case KOOPA_RVT_GET_ELEM_PTR:
    {
        std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&store.dest->kind] << std::endl;
        std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
        std::cout << "  lw " << regs[nreg + 1] << ", (" << regs[nreg + 1] << ")" << std::endl;
        std::cout << "  sw " << reg << ", (" << regs[nreg + 1] << ")" << std::endl;
        break;
    }
    default:
    {
        std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&store.dest->kind] << std::endl;
        std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
        std::cout << "  sw " << reg << ", (" << regs[nreg + 1] << ")" << std::endl;
        break;
    }
    }
}

void InstrHandler::load_handler(const koopa_raw_value_kind_t &kind)
{
    assert(nreg < NREG);
    auto &load = kind.data.load;

    switch (load.src->kind.tag)
    {
    case KOOPA_RVT_ALLOC:
    case KOOPA_RVT_BINARY:
    {
        assert(stk_offsets.count((uintptr_t)&load.src->kind) != 0);
        std::cout << "  lw " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&load.src->kind] << "(sp)" << std::endl;
        break;
    }
    case KOOPA_RVT_GLOBAL_ALLOC:
    {
        std::cout << "  la " << regs[nreg] << ", " << load.src->name + 1 << std::endl;
        std::cout << "  lw " << regs[nreg] << ", (" << regs[nreg] << ")" << std::endl;
        break;
    }
    case KOOPA_RVT_GET_ELEM_PTR:
    {
        std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&load.src->kind] << std::endl;
        std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
        std::cout << "  lw " << regs[nreg + 1] << ", (" << regs[nreg + 1] << ")" << std::endl;
        std::cout << "  lw " << regs[nreg] << ", (" << regs[nreg + 1] << ")" << std::endl;
        break;
    }
    default:
    {
        std::cerr << "unsupported load.src type " << load.src->kind.tag << std::endl;
        assert(false);
        break;
    }
    }

    stk_offsets[(uintptr_t)&kind] = max_offset;
    max_offset += 4;
    std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&kind] << std::endl;
    std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
    std::cout << "  sw " << regs[nreg] << ", (" << regs[nreg + 1] << ")" << std::endl;
}

void InstrHandler::branch_handler(const koopa_raw_value_kind_t &kind)
{
    assert(kind.tag == KOOPA_RVT_BRANCH);
    auto &branch = kind.data.branch;
    std::cout << "  li " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&branch.cond->kind] << std::endl;
    std::cout << "  add " << regs[nreg] << ", " << regs[nreg] << ", sp" << std::endl;
    std::cout << "  lw " << regs[nreg] << ", (" << regs[nreg] << ")" << std::endl;
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
    std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&kind] << std::endl;
    std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
    std::cout << "  sw a0, (" << regs[nreg + 1] << ")" << std::endl;

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

    switch (init->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        std::cout << "  .word " << init->kind.data.integer.value << std::endl;
        break;
    }
    case KOOPA_RVT_ZERO_INIT:
    {
        if (init->ty->tag == KOOPA_RTT_INT32)
            std::cout << "  .zero 4" << std::endl;
        else if (init->ty->tag == KOOPA_RTT_ARRAY)
            std::cout << "  .zero " << 4 * init->ty->data.array.len << std::endl;
        break;
    }
    case KOOPA_RVT_AGGREGATE:
    {
        auto &elem = init->kind.data.aggregate.elems;
        for (int i = 0; i < elem.len; ++i)
            std::cout << "  .word " << ((koopa_raw_value_t)elem.buffer[i])->kind.data.integer.value << std::endl;
        break;
    }
    default:
    {
        std::cerr << "unsupported init kind " << init->kind.tag << std::endl;
        assert(false);
        break;
    }
    }
}

void InstrHandler::getelemptr_handler(const koopa_raw_value_kind_t &kind)
{
    auto &getelemptr = kind.data.get_elem_ptr;
    // std::cerr << "src tag is " << kind.data.get_elem_ptr.src->kind.tag << std::endl;
    if (stk_offsets.count((uintptr_t)&getelemptr.src->kind) == 0)
    {
        stk_offsets[(uintptr_t)&getelemptr.src->kind] = max_offset;
        // assert(getelemptr.src->ty->tag == KOOPA_RTT_ARRAY);
        std::cerr << getelemptr.src->ty->tag << std::endl;
        max_offset += 4 * getelemptr.src->ty->data.pointer.base->data.array.len;
    }

    switch (getelemptr.src->kind.tag)
    {
    case KOOPA_RVT_ALLOC:
    {
        std::cout << "  li " << regs[nreg] << ", " << stk_offsets[(uintptr_t)&getelemptr.src->kind] << std::endl;
        std::cout << "  add " << regs[nreg] << ", " << regs[nreg] << ", sp" << std::endl;
        break;
    }
    case KOOPA_RVT_GLOBAL_ALLOC:
    {
        std::cout << "  la " << regs[nreg] << ", " << getelemptr.src->name + 1 << std::endl;
        break;
    }
    default:
    {
        std::cerr << "unsupported getelemptr src tag " << getelemptr.src->kind.tag << std::endl;
        assert(false);
        break;
    }
    }

    switch (getelemptr.index->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        std::cout << "  li " << regs[nreg + 1] << ", " << getelemptr.index->kind.data.integer.value << std::endl;
        break;
    }
    case KOOPA_RVT_BINARY:
    case KOOPA_RVT_LOAD:
    case KOOPA_RVT_CALL:
    {
        std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&getelemptr.index->kind] << std::endl;
        std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
        std::cout << "  lw " << regs[nreg + 1] << ", (" << regs[nreg + 1] << ")" << std::endl;
        break;
    }
    default:
    {
        std::cerr << "unsupported getelemptr index tag " << getelemptr.index->kind.tag << std::endl;
        assert(false);
        break;
    }
    }

    std::cout << "  li " << regs[nreg + 2] << ", 4" << std::endl;
    std::cout << "  mul " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", " << regs[nreg + 2] << std::endl;
    std::cout << "  add " << regs[nreg] << ", " << regs[nreg] << ", " << regs[nreg + 1] << std::endl;

    stk_offsets[(uintptr_t)&kind] = max_offset;
    max_offset += 4;
    std::cout << "  li " << regs[nreg + 1] << ", " << stk_offsets[(uintptr_t)&kind] << std::endl;
    std::cout << "  add " << regs[nreg + 1] << ", " << regs[nreg + 1] << ", sp" << std::endl;
    std::cout << "  sw " << regs[nreg] << ", (" << regs[nreg + 1] << ")" << std::endl;
}
