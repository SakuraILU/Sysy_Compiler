#pragma once

#include <iostream>
#include <assert.h>

class StmtId
{
public:
    int id = 0;
    struct Node
    {
        int id;
        Node *pre;
        Node(int _id, Node *_pre) : id(_id), pre(_pre) {}
    };

    Node *cur_node = nullptr;

public:
    void push_stmt()
    {
        cur_node = new Node(id++, cur_node);
    }

    void pop_stmt()
    {
        assert(cur_node != nullptr);
        Node *tmp = cur_node;
        cur_node = cur_node->pre;
        delete tmp;
    }

    int get_curid() { return cur_node->id; }
};