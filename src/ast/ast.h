#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <assert.h>
#include "vartable.h"
#include "stmtid.h"

static int nhold = 0;
static StmtId nif;
static StmtId nwhile;

static bool surestop = false;

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
        // std::cout << std::endl;
    }
};

class Stmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override
    {
        if (stmt == nullptr)
            return;

        if (surestop)
            return;

        stmt->Dump();
    }
};

class ReturnStmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
        if (expr != nullptr)
            expr->Dump();
        std::cout << "  ret %" << nhold - 1 << std::endl;
        surestop = true;
    }
};

class AssignStmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
        expr->Dump();
        if (lval != nullptr)
            lval->Dump();
    }
};

class IfStmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> cond;
    std::unique_ptr<BaseAST> if_stmt;
    std::unique_ptr<BaseAST> else_stmt;
    void Dump() const override
    {
        nif.push_stmt();

        int ifret = false, elseret = false;
        cond->Dump();
        if (else_stmt != nullptr)
            std::cout << "  br %" << nhold - 1 << ", \%if_if_then_" << nif.get_curid() << ", \%if_else_" << nif.get_curid() << std::endl;
        else
            std::cout << "  br %" << nhold - 1 << ", \%if_if_then_" << nif.get_curid() << ", \%if_end_" << nif.get_curid() << std::endl;
        std::cout << "%if_if_then_" << nif.get_curid() << ":" << std::endl;
        if_stmt->Dump();
        ifret = surestop;
        if (surestop)
            surestop = false;
        else
            std::cout << "  jump \%if_end_" << nif.get_curid() << std::endl;

        if (else_stmt != nullptr)
        {
            std::cout << "\%if_else_" << nif.get_curid() << ": " << std::endl;
            else_stmt->Dump();
            elseret = surestop;
            if (surestop)
                surestop = false;
            else
                std::cout << "  jump \%if_end_" << nif.get_curid() << std::endl;
        }

        if (else_stmt != nullptr)
        {
            if (ifret && elseret)
            {
                surestop = true;
                nif.pop_stmt();
                return;
            }
        }
        std::cout << "\%if_end_" << nif.get_curid() << ":" << std::endl;
        nif.pop_stmt();
    }
};

class WhileStmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> cond;
    std::unique_ptr<BaseAST> body;
    void Dump() const override
    {
        nwhile.push_stmt();

        std::cout << "  jump %while_entry_" << nwhile.get_curid() << std::endl;
        std::cout << "%while_entry_" << nwhile.get_curid() << ":" << std::endl;
        cond->Dump();
        int cond = nhold - 1;
        std::cout << "  br %" << cond << ", %while_body_" << nwhile.get_curid() << ", \%while_end_" << nwhile.get_curid() << std::endl;

        std::cout << "%while_body_" << nwhile.get_curid() << ":" << std::endl;
        body->Dump();
        if (!surestop)
            std::cout << "  jump %while_entry_" << nwhile.get_curid() << std::endl;
        else
            surestop = false;

        std::cout << "\%while_end_" << nwhile.get_curid() << ":" << std::endl;

        nwhile.pop_stmt();
    }
};

class ContinueStmt : public BaseAST
{
public:
    void Dump() const override
    {
        surestop = true;
        std::cout << "  jump %while_entry_" << nwhile.get_curid() << std::endl;
    }
};

class BreakStmt : public BaseAST
{
public:
    void Dump() const override
    {
        surestop = true;
        std::cout << "  jump %while_end_" << nwhile.get_curid() << std::endl;
    }
};

class Number : public BaseAST
{
public:
    int num;
    void Dump() const override
    {
        std::cout << "  %" << nhold << " = add " << num << ", 0" << std::endl;
        ++nhold;
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
            std::cout << "  %" << nhold << " = sub 0, %" << nhold - 1 << std::endl;
        else if (unary_op.compare("!") == 0)
            std::cout << "  %" << nhold << " = eq %" << nhold - 1 << ", 0" << std::endl;
        else
            assert(false);

        ++nhold;
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
        int rreg = nhold - 1;

        if (mul_op.empty())
            return;

        mul_expr->Dump();
        int lreg = nhold - 1;

        if (mul_op.compare("*") == 0)
            std::cout << "  %" << nhold << " = mul %" << lreg << ", %" << rreg << std::endl;
        else if (mul_op.compare("/") == 0)
            std::cout << "  %" << nhold << " = div %" << lreg << ", %" << rreg << std::endl;
        else if (mul_op.compare("%") == 0)
            std::cout << "  %" << nhold << " = mod %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nhold;
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
        int rreg = nhold - 1;

        if (add_op.empty())
            return;

        add_expr->Dump();
        int lreg = nhold - 1;

        if (add_op.compare("+") == 0)
            std::cout << "  %" << nhold << " = add %" << lreg << ", %" << rreg << std::endl;
        else if (add_op.compare("-") == 0)
            std::cout << "  %" << nhold << " = sub %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nhold;
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
        int rreg = nhold - 1;

        if (rel_op.empty())
            return;

        rel_expr->Dump();
        int lreg = nhold - 1;

        if (rel_op.compare("<") == 0)
            std::cout << "  %" << nhold << " = lt %" << lreg << ", %" << rreg << std::endl;
        else if (rel_op.compare(">") == 0)
            std::cout << "  %" << nhold << " = gt %" << lreg << ", %" << rreg << std::endl;
        else if (rel_op.compare("<=") == 0)
            std::cout << "  %" << nhold << " = le %" << lreg << ", %" << rreg << std::endl;
        else if (rel_op.compare(">=") == 0)
            std::cout << "  %" << nhold << " = ge %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nhold;
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
        int rreg = nhold - 1;

        if (eq_op.empty())
            return;

        eq_expr->Dump();
        int lreg = nhold - 1;

        if (eq_op.compare("==") == 0)
            std::cout << "  %" << nhold << " = eq %" << lreg << ", %" << rreg << std::endl;
        else if (eq_op.compare("!=") == 0)
            std::cout << "  %" << nhold << " = ne %" << lreg << ", %" << rreg << std::endl;
        else
            assert(false);

        ++nhold;
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
    std::unique_ptr<BaseAST> land_expr;
    std::unique_ptr<BaseAST> eq_expr;
    void Dump() const override
    {
        if (land_expr == nullptr)
        {
            eq_expr->Dump();
            return;
        }

        nif.push_stmt();

        land_expr->Dump();
        int lreg = nhold - 1;
        int lout = (nhold++);
        std::cout << "  %" << lout << " = ne %" << lreg << ", 0" << std::endl;
        std::cout << "  @andresult__" << nif.get_curid() << " = alloc i32" << std::endl;
        std::cout << "  store %" << lout << ", @andresult__" << nif.get_curid() << std::endl;
        std::cout << "  br %" << lout << ", \%if_then_" << nif.get_curid() << ", \%if_end_" << nif.get_curid() << std::endl;
        std::cout << "%if_then_" << nif.get_curid() << ":" << std::endl;

        eq_expr->Dump();
        int rreg = nhold - 1;
        int rlout = (nhold++);
        std::cout << "  %" << rlout << " = ne %" << rreg << ", 0" << std::endl;
        std::cout << "  %" << nhold++ << " = and %" << lout << ", %" << rlout << std::endl;
        std::cout << "  store %" << nhold - 1 << ", @andresult__" << nif.get_curid() << std::endl;
        std::cout << "  jump \%if_end_" << nif.get_curid() << std::endl;

        std::cout << "\%if_end_" << nif.get_curid() << ":" << std::endl;
        std::cout << "  %" << nhold++ << " = load @andresult__" << nif.get_curid() << std::endl;

        nif.pop_stmt();
    }

    int Calc() override
    {
        if (land_expr != nullptr)
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
    std::unique_ptr<BaseAST> lor_expr;
    std::unique_ptr<BaseAST> land_expr;
    void Dump() const override
    {
        if (lor_expr == nullptr)
        {
            land_expr->Dump();
            return;
        }

        nif.push_stmt();

        lor_expr->Dump();
        int lreg = nhold - 1;
        int lout = (nhold++);
        std::cout << "  %" << lout << " = ne %" << lreg << ", 0" << std::endl;
        std::cout << "  @orresult__" << nif.get_curid() << " = alloc i32" << std::endl;
        std::cout << "  store %" << lout << ", @orresult__" << nif.get_curid() << std::endl;
        std::cout << "  br %" << lout << ", \%if_end_" << nif.get_curid() << ", \%if_then_" << nif.get_curid() << std::endl;
        std::cout << "%if_then_" << nif.get_curid() << ":" << std::endl;

        land_expr->Dump();
        int rreg = nhold - 1;
        int rlout = (nhold++);
        std::cout << "  %" << rlout << " = ne %" << rreg << ", 0" << std::endl;
        std::cout << "  %" << nhold++ << " = or %" << lout << ", %" << rlout << std::endl;
        std::cout << "  store %" << nhold - 1 << ", @orresult__" << nif.get_curid() << std::endl;
        std::cout << "  jump \%if_end_" << nif.get_curid() << std::endl;

        std::cout << "\%if_end_" << nif.get_curid() << ":" << std::endl;
        std::cout << "  %" << nhold++ << " = load @orresult__" << nif.get_curid() << std::endl;

        nif.pop_stmt();
    }

    int Calc() override
    {
        if (lor_expr != nullptr)
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

        std::cout << "  store %" << nhold - 1 << ", @" << entry.ident << std::endl;
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
            std::cout << "  %" << nhold++ << " = load @" << entry.ident << std::endl;
        else if (entry.type == LocalVarTable::CONST)
            std::cout << "  %" << nhold++ << " = add " << entry.data << ", 0" << std::endl;
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
            std::cout << "  store %" << nhold - 1 << ", @" << actual_ident << std::endl;
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
