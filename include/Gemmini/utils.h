
#ifndef GEMMINI_UTILS_H
#define GEMMINI_UTILS_H
#include <ilang/ilang++.h>

namespace ilang {

namespace Gemmini {
extern ExprRef GetSlice(ExprRef &src, ExprRef idx_hi, int length);
extern ExprRef SetSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high);
extern ExprRef AccSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high);
extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int addresses);
extern ExprRef StoreMulti(ExprRef &memory, ExprRef &to_store, ExprRef &start_addr);
extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max);
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_maximums);
extern ExprRef CastAccTypeToInputType(ExprRef &accTypeElmt);
extern ExprRef ReLUCast(ExprRef &accTypeElmt);

}
}

#endif