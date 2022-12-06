#include <iostream>
#include "gen.h"
#include <assert.h>

CodeGen::CodeGen() : instr_handler(InstrHandler()){};

void CodeGen::generate(const char *koopa_str)
{
    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(koopa_str, &program);
    assert(ret == KOOPA_EC_SUCCESS); // 确保解析时没有出错
    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);

    // 处理 raw program
    // ...
    std::cout << "  .text" << std::endl;
    traverse(raw.funcs);

    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);
}

void CodeGen::traverse(const koopa_raw_slice_t &slice)
{
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            handle(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            handle(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令
            handle(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);
        }
    }
}

void CodeGen::handle(const koopa_raw_function_t &func)
{
    instr_handler.reset();
    std::cout << "  .global " << func->name + 1 << std::endl;
    std::cout << func->name + 1 << ":" << std::endl;
    std::cout << "  addi sp, sp, -256" << std::endl;
    traverse(func->bbs);
}

void CodeGen::handle(const koopa_raw_basic_block_t &bb)
{
    if (bb->name != nullptr)
        std::cout << bb->name + 1 << ":" << std::endl;
    traverse(bb->insts);
}

void CodeGen::handle(const koopa_raw_value_t &value)
{
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    // std::cerr << "value is " << (uintptr_t)value << std::endl;
    // std::cerr << "&kind is " << (uintptr_t)&value->kind << std::endl;
    // std::cerr << "instr tag is " << kind.tag << std::endl;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
    {
        // 访问 return 指令
        instr_handler.ret_handler(kind);
        break;
    }
    case KOOPA_RVT_BINARY:
    {
        // 访问二进制运算指令
        instr_handler.binary_handler(kind);
        break;
    }
    case KOOPA_RVT_INTEGER:
    {
        // 访问 integer 指令
        // Visit(kind.data.integer);
        assert(false);
        break;
    }
    case KOOPA_RVT_STORE:
    {
        instr_handler.store_handler(kind);
        break;
    }
    case KOOPA_RVT_LOAD:
    {
        instr_handler.load_handler(kind);
        break;
    }
    case KOOPA_RVT_ALLOC:
    {
        break;
    }
    case KOOPA_RVT_BRANCH:
    {
        instr_handler.branch_handler(kind);
        break;
    }
    case KOOPA_RVT_JUMP:
    {
        instr_handler.jump_handler(kind);
        break;
    }
    default:
    {
        // 其他类型暂时遇不到
        std::cerr << "unsupported tag " << kind.tag << std::endl;
        assert(false);
        break;
    }
    }
    std::cout << std::endl;
}