#include <Gemmini/Gemmini.h>

namespace ilang {

namespace Gemmini {


extern ExprRef GetSlice(ExprRef &src, ExprRef idx_hi, int length){
    
    // Shift to get rid of low bits
    ExprRef idx_lo = idx_hi - BvConst(length -1,idx_hi.bit_width());
    ExprRef slice = src  >>  idx_lo.ZExt(src.bit_width());

    // Mask out high bits
    auto mask = BvConst(0, src.bit_width());
    auto one  = BvConst(1, src.bit_width());
    for(int i = 0; i < length; i++){
        auto i_bv = BvConst(i, src.bit_width());
        mask = mask | (one << i_bv); 
    }
    slice = slice & mask;
    
    return Extract(slice, length - 1, 0);
}

// Could also implement this by 1) masking out slice range 2) bitwise or with slice
extern ExprRef SetSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high){
    
    ExprRef start_index_low = start_index_high - BvConst(src_bv.bit_width() - 1, start_index_high.bit_width());

    auto mask = BvConst(0, dest_bv.bit_width());
    auto one  = BvConst(1, dest_bv.bit_width());
    for(int i = 0; i < src_bv.bit_width(); i++){
        auto i_bv = BvConst(i, dest_bv.bit_width());
        mask = mask | (one << i_bv); 
    }
    mask = mask << start_index_low.ZExt(mask.bit_width());
    mask = ~mask;
    
    auto to_return = dest_bv & mask; 
    to_return = to_return | (src_bv.ZExt(mask.bit_width()) << start_index_low.ZExt(mask.bit_width()));
    return to_return;
    
}

extern ExprRef AccSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high){
    
    // Compute slice to store
    auto dest_slice = GetSlice(dest_bv, start_index_high, src_bv.bit_width());
    auto to_store   = dest_slice + src_bv;

    return SetSlice(dest_bv, to_store, start_index_high);
    
}

extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max){

    auto unwrapped_result = num1 + num2;
    return Ite(unwrapped_result >= max, unwrapped_result - max, unwrapped_result);
}


extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int addresses){
        // Load the first byte of data
    auto src_elmt = Load(memory, addr);

    // Load the rest of the bytes
    for(int i = 1; i < addresses; i++){
        src_elmt = Concat(src_elmt, Load(memory, addr+i));
        
    }
    return src_elmt;

}


}
}