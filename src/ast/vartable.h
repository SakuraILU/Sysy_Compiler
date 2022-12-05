#include <unordered_map>
#include <string>
#include <iostream>
#include <assert.h>

class LocalVarTable
{
private:
    struct Var
    {
        int type;
        int data;
    };

    struct TableNode
    {
        int bid;
        TableNode *pre;
        std::unordered_map<std::string, Var> val_table;

        TableNode(int _bid, TableNode *_pre = nullptr) : bid(_bid), pre(_pre), val_table(std::unordered_map<std::string, Var>()) {}
    };

    int bid;
    TableNode *cur_table_node;

public:
    enum
    {
        CONST,
        VARIABLE,
    };

    struct Entry
    {
        std::string ident;
        int type;
        int data;
    };

    LocalVarTable() { bid = 0; }

    Entry find_entry(std::string ident)
    {
        TableNode *itr = cur_table_node;
        while (itr != nullptr)
        {
            std::string actual_ident = cvt2acutal_ident(ident, itr->bid);
            // std::cerr << actual_ident << std::endl;
            if (itr->val_table.count(actual_ident))
                return {.ident = actual_ident, .type = itr->val_table[actual_ident].type, .data = itr->val_table[actual_ident].data};
            itr = itr->pre;
        }
        assert(false);
    }

    void add_entry(Entry entry)
    {
        std::string actual_ident = cvt2acutal_ident(entry.ident);

        assert(cur_table_node->val_table.count(actual_ident) == 0);

        cur_table_node->val_table[actual_ident] = {.type = entry.type, .data = entry.data};
    }

    std::string cvt2acutal_ident(std::string ident, int bid = -1)
    {
        if (bid == -1)
            bid = cur_table_node->bid;
        return ident + "_" + std::to_string(bid);
    }

    void push_table()
    {
        cur_table_node = new TableNode(bid++, cur_table_node);
    }

    void pop_table()
    {
        assert(cur_table_node != nullptr);
        TableNode *tmp = cur_table_node;
        cur_table_node = cur_table_node->pre;
        // std::cerr << "add table " << bid << std::endl;
        delete tmp;
    }

    int get_curbid() { return bid; }
};