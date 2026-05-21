#ifndef SIMD_H
#define SIMD_H
// SSE family
#include <emmintrin.h>

#define ExtractFloat(variable, index) (((f32 *)&(variable))[(index)])
#define ExtractInt(variable, index) (((i32 *)&(variable))[(index)])

// Variables
#define wide_f32 __m128
#define wide_i32 __m128i

// Type casting
#define WideCastFloatToInt(a) _mm_castps_si128(a)

// Math
#define WideFloatAdd(a, b) _mm_add_ps((a), (b))
#define WideFloatSubtract(a, b) _mm_sub_ps((a), (b))
#define WideFloatMultiply(a, b) _mm_mul_ps((a), (b))
#define WideFloatDivide(a, b) _mm_div_ps((a), (b))

#define WideFloatSqrt(a) _mm_sqrt_ps(a)
#define WideFloatInvertSign(a) _mm_sub_ps(_mm_set1_ps(0.0f),a)
#define WideFloatSquare(a) WideFloatMultiply(a, a)

// Set
#define WideFloatSetAll(a) _mm_set1_ps(a)
#define WideFloatSetIndividual(d, c, b, a) _mm_set_ps((d), (c), (b), (a))
#define WideIntSetAll(a) _mm_set1_epi32(a)
#define WideIntSetIndividual(d, c, b, a) _mm_set_epi32((d), (c), (b), (a))

// Comparison
#define WideFloatGreater(a, b) _mm_cmpgt_ps((a), (b))
#define WideFloatLess(a, b) _mm_cmplt_ps((a), (b))
#define WideFloatNotGreater(a, b) _mm_cmpngt_ps((a), (b))
#define WideFloatNotLess(a, b) _mm_cmpnlt_ps((a), (b))
#define WideIntTestAllZeros(mask, a) _mm_test_all_zeros((mask), (a))
#define WideIntTestAllOnes(a) _mm_test_all_ones(a)

// Boolean
#define WideFloatOr(a, b) _mm_or_ps((a), (b))
#define WideFloatAnd(a, b) _mm_and_ps((a), (b))

#define WideIntOr(a, b) _mm_or_si128((a), (b))
#define WideIntAnd(a, b) _mm_and_si128((a), (b))

#endif //SIMD_H