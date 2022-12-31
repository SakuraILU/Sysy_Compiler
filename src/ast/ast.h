#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <assert.h>
#include "symtable.h"
#include "stmtid.h"
#include "rparams.h"
#include <vector>

static void import_sysfun();

static int nhold = 0;
static StmtId nif;
static StmtId nwhile;

static bool surestop = false;

static RParams rparms;

static std::vector<int> arr_inits;

static SymTable vartable;

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

    virtual void Dump2() const
    {
        std::cerr << "unimplemented" << std::endl;
        assert(false);
    }
};

class CompUnitAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> comp_unit;
    void Dump() const override
    {
        vartable.push_table();
        import_sysfun();
        comp_unit->Dump();
        vartable.pop_table();
    }
};

class MultCompUnitAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> comp_units;
    std::unique_ptr<BaseAST> comp_unit;
    void Dump() const override
    {
        comp_unit->Dump();
        if (comp_units != nullptr)
            comp_units->Dump();
    }
};

// CompUnit 是 BaseAST
class SingleCompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> comp_unit;

    void Dump() const override
    {
        comp_unit->Dump();
    }
};

class BType : public BaseAST
{
public:
    std::string type;
    void Dump() const override
    {
        assert(type.compare("int") == 0);
        std::cout << "i32";
    }

    void Dump2() const override
    {
        assert(type.compare("int") == 0);
        std::cout << ": i32";
    }
};

class VoidType : public BaseAST
{
public:
    void Dump() const override
    {
        std::cout << "" << std::endl;
    }

    void Dump2() const override
    {
        std::cout << "" << std::endl;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> funcfparams;
    bool hasreturn;
    std::unique_ptr<BaseAST> block;

    void Dump() const override
    {
        SymTable::Entry entry = {.ident = ident, .type = (hasreturn) ? SymTable::FUNC : SymTable::VOIDFUNC};
        vartable.add_entry(entry);

        vartable.push_table();
        std::cout << "fun @" << ident << "(";
        if (funcfparams != nullptr)
            funcfparams->Dump();
        std::cout << ") ";
        func_type->Dump2();
        std::cout << "{ " << std::endl;
        std::cout << "\%" << ident << "_entry:" << std::endl;
        if (funcfparams != nullptr)
            funcfparams->Dump2();
        block->Dump();
        if (!surestop)
            std::cout << "  ret" << std::endl;
        std::cout << "}" << std::endl;

        surestop = false;

        vartable.pop_table();
    }
};

class FuncFParams : public BaseAST
{
public:
    std::unique_ptr<BaseAST> funcfparam;
    std::unique_ptr<BaseAST> funcfparams;
    void Dump() const override
    {
        // std::cerr << "this bid is " << vartable.get_curbid() << std::endl;
        funcfparam->Dump();
        if (funcfparams != nullptr)
        {
            std::cout << ", ";
            funcfparams->Dump();
        }
    }

    void Dump2() const override
    {
        funcfparam->Dump2();
        if (funcfparams != nullptr)
            funcfparams->Dump2();
    }
};

class FuncFParam : public BaseAST
{
public:
    std::unique_ptr<BaseAST> btype;
    std::string ident;
    void Dump() const override
    {
        std::string ident = vartable.cvt2acutal_ident(this->ident);
        std::cout << "%" << ident << ": ";
        btype->Dump();
    }

    void Dump2() const override
    {
        SymTable::Entry entry = {.ident = ident, .type = SymTable::VARIABLE};
        vartable.add_entry(entry);

        std::string ident = vartable.cvt2acutal_ident(this->ident);
        std::cout << "  @" << ident << " = alloc ";
        btype->Dump();
        std::cout << std::endl;

        std::cout << "  store %" << ident << ", @" << ident << std::endl;
    }
};

class FuncRParams : public BaseAST
{
public:
    std::unique_ptr<BaseAST> expr;
    std::unique_ptr<BaseAST> funcrparams;
    void Dump() const override
    {
        expr->Dump();
        rparms.add_param(nhold - 1);
        if (funcrparams != nullptr)
            funcrparams->Dump();
    }

    void Dump2() const override
    {
        std::vector<int> nholds = rparms.get_params();
        for (int i = 0; i < nholds.size() - 1; ++i)
            std::cout << "%" << nholds[i] << ", ";
        std::cout << "%" << nholds[nholds.size() - 1];
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
        vartable.push_table();
        // std::cerr << "alloc table " << vartable.get_curbid() << std::endl;
        block_items->Dump();

        vartable.pop_table();
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
        if (surestop)
            return;

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
        {
            expr->Dump();
            std::cout << "  ret %" << nhold - 1 << std::endl;
        }
        else
            std::cout << "  ret" << std::endl;
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
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
        expr->Dump();
    }

    int Calc() override { return expr->Calc(); }
};

class FuncCall : public BaseAST
{
public:
    std::string ident;
    std::unique_ptr<BaseAST> funcrparams;
    void Dump() const override
    {
        // std::cerr << "funccall" << std::endl;
        SymTable::Entry entry = vartable.find_entry(ident);
        assert(entry.type == SymTable::FUNC || entry.type == SymTable::VOIDFUNC);

        rparms.push();

        if (funcrparams != nullptr)
            funcrparams->Dump();

        if (entry.type == SymTable::FUNC)
            std::cout << "  %" << nhold++ << " = call @" << ident << "(";
        else
            std::cout << "  call @" << ident << "(";

        if (funcrparams != nullptr)
            funcrparams->Dump2();

        std::cout << ")" << std::endl;

        rparms.pop();
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

class Exps : public BaseAST
{
public:
    std::unique_ptr<BaseAST> expr;
    std::unique_ptr<BaseAST> exprs;
    void Dump() const override {}

    int Calc() override
    {
        arr_inits.push_back(expr->Calc());
        if (exprs != nullptr)
            exprs->Calc();
        return arr_inits.size();
    }
};
class LVal : public BaseAST
{
public:
    std::string ident;
    std::unique_ptr<BaseAST> expr;
    void Dump() const override
    {
        SymTable::Entry entry = vartable.find_entry(this->ident);

        // assert(entry.type == SymTable::VARIABLE);

        switch (entry.type)
        {
        case SymTable::VARIABLE:
        {
            std::cout << "  store %" << nhold - 1 << ", @" << entry.ident << std::endl;
            break;
        }
        case SymTable::ARR:
        {
            int nhold_src = nhold - 1;
            expr->Dump();
            std::cout << "  %" << nhold << " = getelemptr @" << entry.ident << ", %" << nhold - 1 << std::endl;
            std::cout << "  store %" << nhold_src << ", %" << nhold << std::endl;
            ++nhold;
            break;
        }
        default:
            assert(false);
            break;
        }
    }
};

class RVal : public BaseAST
{
public:
    std::string ident;
    std::unique_ptr<BaseAST> expr;

    void Dump() const override
    {
        SymTable::Entry entry = vartable.find_entry(this->ident);

        switch (entry.type)
        {
        case SymTable::VARIABLE:
        {
            std::cout << "  %" << nhold++ << " = load @" << entry.ident << std::endl;
            break;
        }
        case SymTable::CONST:
        {
            std::cout << "  %" << nhold++ << " = add " << entry.data << ", 0" << std::endl;
            break;
        }
        case SymTable::CONSTARR:
        case SymTable::ARR:
        {
            expr->Dump();
            std::cout << "  %" << nhold << " = getelemptr @" << entry.ident << ", %" << nhold - 1 << std::endl;
            ++nhold;
            std::cout << "  %" << nhold << " = load %" << nhold - 1 << std::endl;
            ++nhold;
            break;
        }
        default:
            assert(false);
            break;
        }
    }

    int Calc() override
    {
        SymTable::Entry entry = vartable.find_entry(this->ident);
        // std::cerr << entry.ident << " type " << entry.type << std::endl;

        assert(entry.type == SymTable::CONST);

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

class ConstExps : public BaseAST
{
public:
    std::unique_ptr<BaseAST> const_expr;
    std::unique_ptr<BaseAST> const_exprs;
    void Dump() const override {}

    int Calc() override
    {
        arr_inits.push_back(const_expr->Calc());
        if (const_exprs != nullptr)
            const_exprs->Calc();
        return arr_inits.size();
    }
};

class ConstInitVal : public BaseAST
{
public:
    std::unique_ptr<BaseAST> const_exprs;

    void Dump() const override {}

    int Calc() override
    {
        if (const_exprs == nullptr)
            return 0;
        return const_exprs->Calc();
    }
};

class ConstDef : public BaseAST
{
private:
    void DumpGlobalArrInit() const
    {
        std::cout << "{ ";

        const_initval->Calc();
        if (arr_inits.size() == const_exp->Calc())
        {
            for (int i = 0; i < arr_inits.size() - 1; ++i)
            {
                std::cout << arr_inits[i] << ", ";
            }
            std::cout << arr_inits[arr_inits.size() - 1];
        }
        else if (arr_inits.size() < const_exp->Calc())
        {
            for (int i = 0; i < arr_inits.size(); ++i)
            {
                std::cout << arr_inits[i] << ", ";
            }
            for (int i = arr_inits.size(); i < const_exp->Calc() - 1; ++i)
            {
                std::cout << "0, ";
            }
            std::cout << 0;
        }
        else
            assert(false);

        std::cout << "}" << std::endl;

        arr_inits.clear();
    }

    void DumpLocalArrInit() const
    {
        std::string actual_ident = vartable.cvt2acutal_ident(ident);

        const_initval->Calc();
        for (int i = 0; i < arr_inits.size(); ++i)
        {
            std::cout << "  %" << nhold++ << " = getelemptr @" << actual_ident << ", " << i << std::endl;
            std::cout << "  store " << arr_inits[i] << ", %" << nhold - 1 << std::endl;
        }
        for (int i = arr_inits.size(); i < const_exp->Calc(); ++i)
        {
            std::cout << "  %" << nhold++ << " = getelemptr @" << actual_ident << ", " << i << std::endl;
            std::cout << "  store " << 0 << ", %" << nhold - 1 << std::endl;
        }

        arr_inits.clear();
    }

public:
    std::string ident;
    std::unique_ptr<BaseAST> const_exp;
    std::unique_ptr<BaseAST> const_initval;
    void Dump() const override
    {
        assert(const_initval != nullptr);

        if (const_exp == nullptr)
        {
            SymTable::Entry entry = {.ident = ident, .type = SymTable::CONST, .data = const_initval->Calc()};
            vartable.add_entry(entry);
        }
        else
        {
            SymTable::Entry entry = {.ident = ident, .type = SymTable::CONSTARR};
            vartable.add_entry(entry);
            std::string actual_ident = vartable.cvt2acutal_ident(ident);
            if (vartable.compling_global())
            {
                std::cout << "global @" << actual_ident << " = alloc [i32, " << const_exp->Calc() << "], ";
                DumpGlobalArrInit();
            }
            else
            {
                std::cout << "  @" << actual_ident << " = alloc [i32, " << const_exp->Calc() << "]" << std::endl;
                DumpLocalArrInit();
            }
        }
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
private:
    void DefVariable() const
    {
        SymTable::Entry entry = {.ident = ident, .type = SymTable::VARIABLE};
        vartable.add_entry(entry);

        std::string actual_ident = vartable.cvt2acutal_ident(ident);
        // std::cerr << "is global " << vartable.compling_global() << std::endl;
        if (vartable.compling_global())
        {
            if (initval != nullptr)
                std::cout << "global @" << actual_ident << " = alloc i32, " << initval->Calc() << std::endl;
            else
                std::cout << "global @" << actual_ident << " = alloc i32, zeroinit" << std::endl;
        }
        else
        {
            std::cout << "  @" << actual_ident << " = alloc i32" << std::endl;
            if (initval != nullptr)
            {
                initval->Dump();
                std::cout << "  store %" << nhold - 1 << ", @" << actual_ident << std::endl;
            }
        }
    }

    void DefArray() const
    {
        SymTable::Entry entry = {.ident = ident, .type = SymTable::ARR};
        vartable.add_entry(entry);

        std::string actual_ident = vartable.cvt2acutal_ident(ident);

        if (vartable.compling_global())
        {
            std::cout << "global @" << actual_ident << " = alloc [i32, " << const_exp->Calc() << "], ";
            DumpGlobalArrInit();
        }
        else
        {
            std::cout << "  @" << actual_ident << " = alloc [i32, " << const_exp->Calc() << "]" << std::endl;
            DumpLocalArrInit();
        }
    }

    void DumpGlobalArrInit() const
    {
        if (initval == nullptr)
        {
            std::cout << "zeroinit" << std::endl;
            return;
        }

        std::cout << "{ ";

        initval->Calc();
        if (arr_inits.size() == const_exp->Calc())
        {
            for (int i = 0; i < arr_inits.size() - 1; ++i)
            {
                std::cout << arr_inits[i] << ", ";
            }
            std::cout << arr_inits[arr_inits.size() - 1];
        }
        else if (arr_inits.size() < const_exp->Calc())
        {
            for (int i = 0; i < arr_inits.size(); ++i)
            {
                std::cout << arr_inits[i] << ", ";
            }
            for (int i = arr_inits.size(); i < const_exp->Calc() - 1; ++i)
            {
                std::cout << "0, ";
            }
            std::cout << 0;
        }
        else
            assert(false);

        std::cout << "}" << std::endl;

        arr_inits.clear();
    }

    void DumpLocalArrInit() const
    {
        if (initval == nullptr)
            return;

        std::string actual_ident = vartable.cvt2acutal_ident(ident);

        initval->Calc();
        for (int i = 0; i < arr_inits.size(); ++i)
        {
            std::cout << "  %" << nhold++ << " = getelemptr @" << actual_ident << ", " << i << std::endl;
            std::cout << "  store " << arr_inits[i] << ", %" << nhold - 1 << std::endl;
        }
        for (int i = arr_inits.size(); i < const_exp->Calc(); ++i)
        {
            std::cout << "  %" << nhold++ << " = getelemptr @" << actual_ident << ", " << i << std::endl;
            std::cout << "  store " << 0 << ", %" << nhold - 1 << std::endl;
        }

        arr_inits.clear();
    }

public:
    std::string ident;
    std::unique_ptr<BaseAST> const_exp;
    std::unique_ptr<BaseAST> initval;
    void Dump() const override
    {
        if (const_exp == nullptr)
            DefVariable();
        else
            DefArray();
    }
};

class InitVal : public BaseAST
{
public:
    std::unique_ptr<BaseAST> exprs;
    void Dump() const override
    {
        if (exprs == nullptr)
            return;

        exprs->Dump();
    }

    int Calc() override
    {
        if (exprs == nullptr)
            return 0;

        return exprs->Calc();
    }
};

static void import_sysfun()
{
    std::cout << "decl @getint(): i32" << std::endl;
    SymTable::Entry entry = {.ident = "getint", .type = SymTable::FUNC};
    vartable.add_entry(entry);

    std::cout << "decl @getch(): i32" << std::endl;
    entry = {.ident = "getch", .type = SymTable::FUNC};
    vartable.add_entry(entry);

    std::cout << "decl @getarray(*i32): i32" << std::endl;
    entry = {.ident = "getarray", .type = SymTable::FUNC};
    vartable.add_entry(entry);

    std::cout << "decl @putint(i32)" << std::endl;
    entry = {.ident = "putint", .type = SymTable::VOIDFUNC};
    vartable.add_entry(entry);

    std::cout << "decl @putch(i32)" << std::endl;
    entry = {.ident = "putch", .type = SymTable::VOIDFUNC};
    vartable.add_entry(entry);

    std::cout << "decl @putarray(i32, *i32)" << std::endl;
    entry = {.ident = "putarray", .type = SymTable::VOIDFUNC};
    vartable.add_entry(entry);

    std::cout << "decl @starttime()" << std::endl;
    entry = {.ident = "starttime", .type = SymTable::VOIDFUNC};
    vartable.add_entry(entry);

    std::cout << "decl @stoptime()" << std::endl;
    entry = {.ident = "stoptime", .type = SymTable::VOIDFUNC};
    vartable.add_entry(entry);
}