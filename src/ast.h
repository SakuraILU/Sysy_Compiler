#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <assert.h>

static int nstmt = 0;

// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override
    {
        // std::cout << "CompUnitAST { ";
        func_def->Dump();
        // std::cout << " }" << std::endl;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override
    {
        std::cout << "fun @" << ident << "(): ";
        func_type->Dump();
        block->Dump();
        std::cout << std::endl;
    }
};

class FuncType : public BaseAST
{
public:
    std::string func_type;

    void Dump() const override
    {
        std::cout << "i32 ";
    }
};

class Block : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override
    {
        std::cout << "{ " << std::endl;
        std::cout << "\%entry:" << std::endl;
        stmt->Dump();
        std::cout << "}";
        // std::cout << "BlockAST { ";
        // stmt->Dump();
        // std::cout << " }";
    }
};

class Stmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
        expr->Dump();
        std::cout << "  ret %" << nstmt - 1 << std::endl;
    }
};

class Number : public BaseAST
{
public:
    int num;
    void Dump() const override
    {
        std::cout << "  %" << nstmt << " = sub " << num << ", 0" << std::endl;
        ++nstmt;
    }
};

class Exp : public BaseAST
{
public:
    std::unique_ptr<BaseAST> lor_expr;
    void Dump() const override
    {
        lor_expr->Dump();
    }
};

class PrimaryExp : public BaseAST
{
public:
    // bool is_expr;
    std::unique_ptr<BaseAST> expr_or_num;
    void Dump() const override
    {
        expr_or_num->Dump();
    }
};

class UnaryExp : public BaseAST
{
public:
    std::string unary_op;
    std::unique_ptr<BaseAST> unary_or_p_expr;
    void Dump() const override
    {
        unary_or_p_expr->Dump();

        if (unary_op.empty() || unary_op.compare("+") == 0)
            return;

        if (unary_op.compare("-") == 0)
            std::cout << "  %" << nstmt << " = sub 0, %" << nstmt - 1 << std::endl;
        else if (unary_op.compare("!") == 0)
            std::cout << "  %" << nstmt << " = eq %" << nstmt - 1 << ", 0" << std::endl;
        else
            assert(false);

        ++nstmt;
    }
};

class MulExp : public BaseAST
{
public:
    std::string mul_op;
    std::unique_ptr<BaseAST> mul_expr;
    std::unique_ptr<BaseAST> unary_expr;
    void Dump() const override
    {
        unary_expr->Dump();
        int rreg = nstmt - 1;

        if (mul_op.empty())
            return;

        mul_expr->Dump();
        int lreg = nstmt - 1;

        if (mul_op.compare("*") == 0)
            std::cout << "  %" << nstmt << " = mul %" << lreg << ", %" << rreg << std::endl;
        else if (mul_op.compare("/") == 0)
            std::cout << "  %" << nstmt << " = div %" << lreg << ", %" << rreg << std::endl;
        else if (mul_op.compare("%") == 0)
            std::cout << "  %" << nstmt << " = mod %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nstmt;
    }
};

class AddExp : public BaseAST
{
public:
    std::string add_op;
    std::unique_ptr<BaseAST> add_expr;
    std::unique_ptr<BaseAST> mul_expr;
    void Dump() const override
    {
        mul_expr->Dump();
        int rreg = nstmt - 1;

        if (add_op.empty())
            return;

        add_expr->Dump();
        int lreg = nstmt - 1;

        // std::cerr << "add op is " << add_op << std::endl;
        if (add_op.compare("+") == 0)
            std::cout << "  %" << nstmt << " = add %" << lreg << ", %" << rreg << std::endl;
        else if (add_op.compare("-") == 0)
            std::cout << "  %" << nstmt << " = sub %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nstmt;
    }
};

class RelExp : public BaseAST
{
public:
    std::string rel_op;
    std::unique_ptr<BaseAST> rel_expr;
    std::unique_ptr<BaseAST> add_expr;
    void Dump() const override
    {
        add_expr->Dump();
        int rreg = nstmt - 1;

        if (rel_op.empty())
            return;

        rel_expr->Dump();
        int lreg = nstmt - 1;

        // std::cerr << "add op is " << add_op << std::endl;
        if (rel_op.compare("<") == 0)
            std::cout << "  %" << nstmt << " = lt %" << lreg << ", %" << rreg << std::endl;
        else if (rel_op.compare(">") == 0)
            std::cout << "  %" << nstmt << " = gt %" << lreg << ", %" << rreg << std::endl;
        else if (rel_op.compare("<=") == 0)
            std::cout << "  %" << nstmt << " = le %" << lreg << ", %" << rreg << std::endl;
        else if (rel_op.compare(">=") == 0)
            std::cout << "  %" << nstmt << " = ge %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nstmt;
    }
};

class EqExp : public BaseAST
{
public:
    std::string eq_op;
    std::unique_ptr<BaseAST> eq_expr;
    std::unique_ptr<BaseAST> rel_expr;
    void Dump() const override
    {
        rel_expr->Dump();
        int rreg = nstmt - 1;

        if (eq_op.empty())
            return;

        eq_expr->Dump();
        int lreg = nstmt - 1;

        // std::cerr << "add op is " << add_op << std::endl;
        if (eq_op.compare("==") == 0)
            std::cout << "  %" << nstmt << " = eq %" << lreg << ", %" << rreg << std::endl;
        else if (eq_op.compare("!=") == 0)
            std::cout << "  %" << nstmt << " = ne %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nstmt;
    }
};

class LAndExp : public BaseAST
{
public:
    std::string land_op;
    std::unique_ptr<BaseAST> land_expr;
    std::unique_ptr<BaseAST> eq_expr;
    void Dump() const override
    {
        eq_expr->Dump();
        int rreg = nstmt - 1;

        if (land_op.empty())
            return;

        land_expr->Dump();
        int lreg = nstmt - 1;

        if (land_op.compare("&&") == 0)
        {
            std::cout << "  %" << nstmt++ << " = ne %" << lreg << ", 0" << std::endl;
            std::cout << "  %" << nstmt++ << " = ne %" << rreg << ", 0" << std::endl;
            std::cout << "  %" << nstmt << " = and %" << nstmt - 1 << ", %" << nstmt - 2 << std::endl;
        }
        else
            assert(false);

        ++nstmt;
    }
};

class LOrExp : public BaseAST
{
public:
    std::string lor_op;
    std::unique_ptr<BaseAST> lor_expr;
    std::unique_ptr<BaseAST> land_expr;
    void Dump() const override
    {
        land_expr->Dump();
        int rreg = nstmt - 1;

        if (lor_op.empty())
            return;

        lor_expr->Dump();
        int lreg = nstmt - 1;

        // std::cerr << "add op is " << add_op << std::endl;
        if (lor_op.compare("||") == 0)
        {
            std::cout << "  %" << nstmt++ << " = ne %" << lreg << ", 0" << std::endl;
            std::cout << "  %" << nstmt++ << " = ne %" << rreg << ", 0" << std::endl;
            std::cout << "  %" << nstmt << " = or %" << nstmt - 1 << ", %" << nstmt - 2 << std::endl;
        }
        else
            assert(false);

        ++nstmt;
    }
};