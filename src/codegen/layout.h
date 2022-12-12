#pragma once

#include <string>

#define STKSIZE 256
#define RASTKOFF 0
#define ARGSTKOFF 4
#define STKOFF 48

static std::string regs[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
#define NREG (sizeof(regs) / sizeof(regs[0]))
#define ARGREG(i) (regs[(i) + 7])
