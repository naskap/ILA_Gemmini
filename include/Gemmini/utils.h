
#ifndef GEMMINI_UTILS_H
#define GEMMINI_UTILS_H
#include <ilang/ilang++.h>

namespace ilang {

namespace Gemmini {
extern ExprRef GetBvSlice(ExprRef const &src, ExprRef const &idx_hi, int length);
extern ExprRef GetBvElmt(ExprRef const &src_bv, ExprRef const &idx, int elmt_size);
extern ExprRef GetMemElmt(ExprRef const &src_mem, ExprRef const &row, ExprRef const &col, int elmt_size);
extern ExprRef SetBvSlice(ExprRef const &dest_bv, ExprRef const &start_index_high, ExprRef const &src_bv);
extern ExprRef SetBvElmt(ExprRef const &dest_bv, ExprRef const &idx, ExprRef const &src_bv);
extern ExprRef SetMemElmt(ExprRef const &dest_mem, ExprRef const &row, ExprRef const &col, ExprRef const &src_bv);
extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int addresses);
extern ExprRef StoreMulti(ExprRef &memory, ExprRef &to_store, ExprRef &start_addr);
extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max);
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_maximums);

extern ExprRef ApplyActivation(ExprRef &src, ExprRef &act, unsigned int out_width);
extern ExprRef CastBv(ExprRef &src, unsigned int out_width);
extern ExprRef ReLUCast(ExprRef &accTypeElmt, unsigned int out_width);

}
}

#endif