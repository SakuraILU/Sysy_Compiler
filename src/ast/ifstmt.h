#pragma once

#include <iostream>
#include <assert.h>

class IFStmtId
{
public:
    int fid = 0;
    struct IFNode
    {
        int fid;
        IFNode *pre;
        IFNode(int _fid, IFNode *_pre) : fid(_fid), pre(_pre) {}
    };

    IFNode *cur_node = nullptr;

public:
    void push_if()
    {
        cur_node = new IFNode(fid++, cur_node);
    }

    void pop_if()
    {
        assert(cur_node != nullptr);
        IFNode *tmp = cur_node;
        cur_node = cur_node->pre;
        delete tmp;
    }

    int get_curfid() { return cur_node->fid; }
};