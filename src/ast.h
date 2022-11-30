#pragma once

#include <iostream>
#include <memory>
#include <string>

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
        // std::cout << "FuncDefAST { ";
        // func_type->Dump();
        // std::cout << ", " << ident << ", ";
        // block->Dump();
        // std::cout << " }";
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
        stmt->Dump();
        std::cout << std::endl
                  << "}";
        // std::cout << "BlockAST { ";
        // stmt->Dump();
        // std::cout << " }";
    }
};

class Stmt : public BaseAST
{
public:
    std::unique_ptr<BaseAST> retval;
    void Dump() const override
    {
        std::cout << "\%entry:" << std::endl
                  << "  ret ";
        retval->Dump();
        // std::cout << "StmtAST { ";
        // retval->Dump();
        // std::cout << " }";
    }
};

class Number : public BaseAST
{
public:
    int num;
    void Dump() const override
    {
        std::cout << num;
    }
};
