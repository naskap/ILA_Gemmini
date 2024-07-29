#include <Gemmini/Gemmini.h>

namespace ilang {

namespace Gemmini {


extern ExprRef GetSlice(ExprRef &src, ExprRef idx_hi, int length){
    
    // Shift to get rid of low bits
    ExprRef idx_lo = idx_hi - BvConst(length -1,idx_hi.bit_width());
    ExprRef slice = src  >>  idx_lo.ZExt(src.bit_width());

    // Mask out high bits
    int mask = 0;
    for(int i = 0; i < length; i++){
        mask |= (1 << i); 
    }
    slice = slice & BvConst(mask, slice.bit_width());
    
    return Extract(slice, length - 1, 0);
}

// Could also implement this by 1) masking out slice range 2) bitwise or with slice
extern ExprRef SetSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high){
    
    auto to_return = BvConst(0,1);  // Workaround since empty bvs are not allowed (will be removed at the end)
    for(int i = 0; i < dest_bv.bit_width(); i++){
        auto i_bv              = BvConst(i,16);
        auto bit_i_is_in_slice = (i_bv <= start_index_high) & (i_bv > (start_index_high - src_bv.bit_width()));
             to_return         = Ite(bit_i_is_in_slice, 
                Concat(to_return, Extract(src_bv,i,i) == 1), 
                Concat(to_return, Extract(dest_bv,i,i) == 1));
    }
    
    return Extract(to_return, to_return.bit_width() - 2,0);
    
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