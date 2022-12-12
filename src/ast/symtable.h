#pragma once

#include <unordered_map>
#include <string>
#include <iostream>
#include <assert.h>

/**
 * @brief Symbol Table for global or local functions or variables.
 * table first pushed is for global,
 * others are for different local blocks, nearest pushed one are the nearest local symtable.
 *
 */
class SymTable
{
private:
    struct Sym
    {
        int type;
        int data;
    };

    struct TableNode
    {
        int bid;
        TableNode *pre;
        std::unordered_map<std::string, Sym> val_table;

        TableNode(int _bid, TableNode *_pre = nullptr) : bid(_bid), pre(_pre), val_table(std::unordered_map<std::string, Sym>()) {}
    };

    int bid;
    TableNode *cur_table_node;

public:
    enum
    {
        CONST,
        VARIABLE,
        FUNC,
        VOIDFUNC,
    };

    /*
        Entry in variable table
        when pass as argument, ident is the name in the src code
        when return, ident is the actual name in the symbol table with the suffix bid
    */
    struct Entry
    {
        std::string ident; // identity in src code. Don't add suffix bid by method cvrt2acutal_ident(string)
        int type;          // enum type: CONST, VARIABLE, FUNC, VOIDFUNC
        int data;          // only entry whith CONST type has data to store, const data will be calculated when initialization
    };

    /**
     * @brief Construct a new Var Table object
     *
     */
    SymTable() { bid = -1; }

    /**
     * @brief return an entry named ident
     *
     * @param ident name of the entry in src code (with no bid)
     * @return Entry with acutal identity name (ident_bid)
     */
    Entry find_entry(std::string ident)
    {
        TableNode *itr = cur_table_node;
        while (itr != nullptr)
        {
            std::string actual_ident = cvt2acutal_ident(ident, itr->bid);
            if (itr->val_table.count(actual_ident))
                return {.ident = actual_ident, .type = itr->val_table[actual_ident].type, .data = itr->val_table[actual_ident].data};
            itr = itr->pre;
        }
        assert(false);
    }

    /**
     * @brief add an entry in variable table
     *
     * @param entry ident in src code, a suffix will be add in the end of the ident that is ident_bid
     */
    void add_entry(Entry entry)
    {
        std::string actual_ident = cvt2acutal_ident(entry.ident);

        assert(cur_table_node->val_table.count(actual_ident) == 0);

        cur_table_node->val_table[actual_ident] = {.type = entry.type, .data = entry.data};
    }

    /**
     * @brief  add suffix num in behind the ident
     *
     * @param ident
     * @param bid set a particular bid, if bid is not assigned, get the nearest identity from the current table
     * @return std::string ident_suffixNum
     */
    std::string cvt2acutal_ident(std::string ident, int bid = -1)
    {
        if (bid == -1)
            bid = cur_table_node->bid;
        return ident + "_" + std::to_string(bid);
    }

    /**
     * @brief push a new table
     *
     */
    void push_table()
    {
        cur_table_node = new TableNode(++bid, cur_table_node);
    }

    /**
     * @brief pop a nearest table
     *
     */
    void pop_table()
    {
        assert(cur_table_node != nullptr);
        TableNode *tmp = cur_table_node;
        cur_table_node = cur_table_node->pre;
        if (cur_table_node != nullptr && cur_table_node->bid == 0)
            bid = 0;
        // std::cerr << "add table " << bid << std::endl;
        // std::cerr << "current bid is " << cur_table_node->bid << std::endl;
        delete tmp;
    }

    /**
     * @brief Get the current bid, used for debugging
     *
     * @return bid
     */
    int get_curbid() { return bid; }

    /**
     * @brief wheather is compiling global object (funcs or variables)
     *
     * @return true on global or false on local
     */
    bool compling_global() { return bid == 0; }
};