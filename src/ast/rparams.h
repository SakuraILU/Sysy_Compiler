#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <assert.h>

/**
 * @brief recurssively store parameters passed to different functions, like mul( add(3, 4), sub(1, mul(3,1) ) )
 *
 */
class RParams
{
public:
    std::stack<std::vector<int>> rparam_stk;

    /**
     * @brief when recussively call a new function, push a new rparm stack
     *
     */
    void push()
    {
        std::vector<int> nholds;
        rparam_stk.push(nholds);
    }

    /**
     * @brief this function is dealt over, pop stack
     *
     */
    void pop()
    {
        rparam_stk.pop();
    }

    /**
     * @brief add new rparm
     *
     * @param nhold the hold num in IR
     */
    void add_param(int nhold)
    {
        rparam_stk.top().push_back(nhold);
    }

    /**
     * @brief Get the params for the top function
     *
     * @return std::vector<int>
     */
    std::vector<int> get_params()
    {
        return rparam_stk.top();
    }
};