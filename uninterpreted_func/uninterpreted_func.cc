#include <Gemmini.h>
#include <systemc.h>
#include <ac_int.h>
#include <ac_std_float.h>
#include <cfenv>

template<class To, class From>
To _bit_cast(const From& src) noexcept
{ 
    To dst;
    std:: memcpy(&dst, &src, sizeof(To));
    return dst;
}


sc_biguint<8> Gemmini::ScaleInputType(sc_biguint<8> input,sc_biguint<32> scale){

  // Bit-cast scale to float
  ac_ieee_float32 scale_ac_float = _bit_cast<float, uint32_t>(scale.to_uint());

  // Convert input to float
  ac_ieee_float32 input_ac_float(_bit_cast<int8_t,uint8_t>(input.to_uint()));

  // Perform multiply
  auto result_ac_float = input_ac_float * scale_ac_float;

  // Cast to int with rounding to the nearest even number
  // TODO: Make clamping programmatic with generated gemmini bitwidths
  // TODO: Use custom types corresponding to the gemmini configuration
  auto   result_float   = result_ac_float.to_float();
  auto   result_int     = static_cast<int64_t>(std::nearbyint(result_float));
  int8_t result_clamped = result_int > INT8_MAX ? INT8_MAX : (result_int < INT8_MIN ? INT8_MIN : result_int);

  // Bit-cast back to sc_biguint8
  uint8_t result_uint = _bit_cast<uint8_t, int8_t>(result_clamped);
  sc_biguint<8> to_return(result_uint);

  return to_return;
}


sc_biguint<32> Gemmini::ScaleAccType(sc_biguint<32> input,sc_biguint<32> scale){
  
  // bitcast scale to float
  ac_ieee_float32 scale_ac_float = _bit_cast<float, uint32_t>(scale.to_uint());

  // Cast scale and input to 64 bit floats
  ac_ieee_float64 scale_ac_float64(scale_ac_float);
  ac_ieee_float64 input_float64(_bit_cast<int32_t,uint32_t>(input.to_uint()));

  // Perform multiply
  ac_ieee_float64 result_float64 = scale_ac_float64 * input_float64;

  // Cast to int with rounding to the nearest integer
  auto result_double  = result_float64.to_double();
  auto result_rounded = std::nearbyint(result_double);
  auto result_clamped = result_rounded > INT32_MAX ? INT32_MAX : (result_rounded < INT32_MIN ? INT32_MIN : result_rounded);
  auto result_int     = static_cast<int32_t>(result_clamped);
  
  // Cast back to sc_biguint
  sc_biguint<32> out(result_int);
  return out;
}

