#include <Gemmini/Gemmini.h>
namespace ilang {

namespace Gemmini {
extern ExprRef CastUnsigned(ExprRef to_cast, int bw){
    return Concat(BvConst(0,bw - to_cast.bit_width()), to_cast);
}

}
}