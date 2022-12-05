#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <assert.h>
#include "vartable.h"

static int nstmt = 0;

static LocalVarTable local_vartable;

// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual int Calc()
    {
        std::cerr << "unimplemented" << std::endl;
        assert(false);
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override
    {
        func_def->Dump();
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
        std::cout << "{ " << std::endl;
        std::cout << "\%entry:" << std::endl;
        block->Dump();
        std::cout << "}";
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
    std::unique_ptr<BaseAST> block_items;
    void Dump() const override
    {
        if (block_items == nullptr)
            return;
        local_vartable.push_table();
        // std::cerr << "alloc table " << local_vartable.get_curbid() << std::endl;
        block_items->Dump();

        local_vartable.pop_table();
    }
};

class BlockItems : public BaseAST
{
public:
    std::unique_ptr<BaseAST> block_item;
    std::unique_ptr<BaseAST> block_items;
    void Dump() const override
    {
        block_item->Dump();
        if (block_items != nullptr)
            block_items->Dump();
    }
};

class BlockItem : public BaseAST
{
public:
    std::unique_ptr<BaseAST> decl_or_stmt;
    void Dump() const override
    {
        decl_or_stmt->Dump();
    }
};

class Stmt : public BaseAST
{
public:
    bool isret = false;
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> expr;
    std::unique_ptr<BaseAST> block;

    void Dump() const override
    {
        if (block != nullptr)
        {
            block->Dump();
            return;
        }

        if (lval == nullptr && expr == nullptr)
            return;
        else if (lval == nullptr)
            expr->Dump();
        else if (lval != nullptr && expr != nullptr)
        {
            expr->Dump();
            lval->Dump();
        }
        else
            assert(false);

        if (isret)
            std::cout << "  ret %" << nstmt - 1 << std::endl;
    }
};

class Number : public BaseAST
{
public:
    int num;
    void Dump() const override
    {
        std::cout << "  %" << nstmt << " = add " << num << ", 0" << std::endl;
        ++nstmt;
    }

    int Calc() override
    {
        return num;
    }
};

class PrimaryExp : public BaseAST
{
public:
    // bool is_expr;
    std::unique_ptr<BaseAST> expr_or_num_or_lval_or_rval;
    void Dump() const override
    {
        expr_or_num_or_lval_or_rval->Dump();
    }

    int Calc() override { return expr_or_num_or_lval_or_rval->Calc(); }
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

    int Calc() override
    {
        if (!unary_op.empty())
            if (unary_op.compare("-") == 0)
                return -unary_or_p_expr->Calc();
            else if (unary_op.compare("!") == 0)
                return !unary_or_p_expr->Calc();
            else if (unary_op.compare("+") == 0)
                return unary_or_p_expr->Calc();
            else
                assert(false);
        else
            return unary_or_p_expr->Calc();
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

    int Calc() override
    {
        if (!mul_op.empty())
            if (mul_op.compare("*") == 0)
                return mul_expr->Calc() * unary_expr->Calc();
            else if (mul_op.compare("/") == 0)
                return mul_expr->Calc() / unary_expr->Calc();
            else if (mul_op.compare("%") == 0)
                return mul_expr->Calc() % unary_expr->Calc();
            else
                assert(false);
        else
            return unary_expr->Calc();
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

        if (add_op.compare("+") == 0)
            std::cout << "  %" << nstmt << " = add %" << lreg << ", %" << rreg << std::endl;
        else if (add_op.compare("-") == 0)
            std::cout << "  %" << nstmt << " = sub %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nstmt;
    }

    int Calc() override
    {
        if (!add_op.empty())
            if (add_op.compare("+") == 0)
                return add_expr->Calc() + mul_expr->Calc();
            else if (add_op.compare("-") == 0)
                return add_expr->Calc() - mul_expr->Calc();
            else
                assert(false);
        else
            return mul_expr->Calc();
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

    int Calc() override
    {
        if (!rel_op.empty())
            if (rel_op.compare("<") == 0)
                return rel_expr->Calc() < add_expr->Calc();
            else if (rel_op.compare(">") == 0)
                return rel_expr->Calc() > add_expr->Calc();
            else if (rel_op.compare("<=") == 0)
                return rel_expr->Calc() <= add_expr->Calc();
            else if (rel_op.compare(">=") == 0)
                return rel_expr->Calc() >= add_expr->Calc();
            else
                assert(false);
        else
            return add_expr->Calc();
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

        if (eq_op.compare("==") == 0)
            std::cout << "  %" << nstmt << " = eq %" << lreg << ", %" << rreg << std::endl;
        else if (eq_op.compare("!=") == 0)
            std::cout << "  %" << nstmt << " = ne %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nstmt;
    }

    int Calc() override
    {
        if (!eq_op.empty())
            if (eq_op.compare("==") == 0)
                return eq_expr->Calc() == rel_expr->Calc();
            else if (eq_op.compare("!=") == 0)
                return eq_expr->Calc() != rel_expr->Calc();
            else
                assert(false);
        else
            return rel_expr->Calc();
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

    int Calc() override
    {
        if (!land_op.empty())
            return land_expr->Calc() && eq_expr->Calc();
        else
        {
            return eq_expr->Calc();
        }
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

    int Calc() override
    {
        if (!lor_op.empty())
            return lor_expr->Calc() || land_expr->Calc();
        else
        {
            return land_expr->Calc();
        }
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

    int Calc() override
    {
        return lor_expr->Calc();
    }
};

class LVal : public BaseAST
{
public:
    std::string ident;
    void Dump() const override
    {
        LocalVarTable::Entry entry = local_vartable.find_entry(this->ident);

        assert(entry.type == LocalVarTable::VARIABLE);

        std::cout << "  store %" << nstmt - 1 << ", @" << entry.ident << std::endl;
    }
};

class RVal : public BaseAST
{
public:
    std::string ident;
    void Dump() const override
    {
        LocalVarTable::Entry entry = local_vartable.find_entry(this->ident);

        // assert(cur_table_node->val_table.count(ident) != 0);

        if (entry.type == LocalVarTable::VARIABLE)
            std::cout << "  %" << nstmt++ << " = load @" << entry.ident << std::endl;
        else if (entry.type == LocalVarTable::CONST)
            std::cout << "  %" << nstmt++ << " = add " << entry.data << ", 0" << std::endl;
    }

    int Calc() override
    {
        LocalVarTable::Entry entry = local_vartable.find_entry(this->ident);

        assert(entry.type == LocalVarTable::CONST);

        return entry.data;
    }
};

class ConstExp : public BaseAST
{
public:
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
    }

    int Calc() override
    {
        return expr->Calc();
    }
};

class ConstInitVal : public BaseAST
{
public:
    std::unique_ptr<BaseAST> const_expr;
    void Dump() const override
    {
    }

    int Calc() override
    {
        return const_expr->Calc();
    }
};

class ConstDef : public BaseAST
{
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_initval;
    void Dump() const override
    {
        assert(const_initval != nullptr);

        LocalVarTable::Entry entry = {.ident = ident, .type = LocalVarTable::CONST, .data = const_initval->Calc()};
        local_vartable.add_entry(entry);
    }
};

class ConstDefs : public BaseAST
{
public:
    std::unique_ptr<BaseAST> const_def;
    std::unique_ptr<BaseAST> const_defs;
    void Dump() const override
    {
        const_def->Dump();
        if (const_defs != nullptr)
            const_defs->Dump();
    }
};

class BType : public BaseAST
{
public:
    std::string type;
    void Dump() const override
    {
        assert(type.compare("int"));
    }
};

class ConstDecl : public BaseAST
{
public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> const_defs;
    void Dump() const override
    {
        const_defs->Dump();
    }
};

class Decl : public BaseAST
{
public:
    std::unique_ptr<BaseAST> decl;
    void Dump() const override
    {
        decl->Dump();
    }
};

class VarDecl : public BaseAST
{
public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> var_defs;
    void Dump() const override
    {
        var_defs->Dump();
    }
};

class VarDefs : public BaseAST
{
public:
    std::unique_ptr<BaseAST> var_def;
    std::unique_ptr<BaseAST> var_defs;
    void Dump() const override
    {
        var_def->Dump();
        if (var_defs != nullptr)
            var_defs->Dump();
    }
};

class VarDef : public BaseAST
{
public:
    std::string ident;
    std::unique_ptr<BaseAST> initval;
    void Dump() const override
    {
        LocalVarTable::Entry entry = {.ident = ident, .type = LocalVarTable::VARIABLE};
        local_vartable.add_entry(entry);

        std::string actual_ident = local_vartable.cvt2acutal_ident(ident);
        std::cout << "  @" << actual_ident << " = alloc i32" << std::endl;
        if (initval != nullptr)
        {
            initval->Dump();
            std::cout << "  store %" << nstmt - 1 << ", @" << actual_ident << std::endl;
        }
    }
};

class InitVal : public BaseAST
{
public:
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
        expr->Dump();
    }
};
