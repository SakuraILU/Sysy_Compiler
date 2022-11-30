#pragma once

#include "koopa.h"
#include <iostream>

class InstrHandler
{
public:
    void ret_handler(koopa_raw_return_t ret)
    {
        std::cout << "  li a0, " << ret.value->kind.data.integer.value << std::endl;
        std::cout << "  ret" << std::endl;
    }
};