/* SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright:
 *   2020      Evan Nemerson <evan@nemerson.com>
 *   2020      Christopher Moore <moore@free.fr>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Ju-Hung Li <jhlee@pllab.cs.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_TBL_H)
#define SIMDE_ARM_NEON_TBL_H

#include "reinterpret.h"
#include "combine.h"
#include "get_low.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vtbl1_u8(simde_uint8x8_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl1_u8(a, b);
  #elif defined(SIMDE_WASM_SIMD128_NATIVE)
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(simde_vcombine_u8(a, a)),
      b_ = simde_uint8x16_to_private(simde_vcombine_u8(b, b));

    r_.v128 = wasm_i8x16_swizzle(a_.v128, b_.v128);
    r_.v128 = wasm_v128_and(r_.v128, wasm_u8x16_lt(b_.v128, wasm_i8x16_splat(8)));

    return simde_vget_low_u8(simde_uint8x16_from_private(r_));
  #else
    simde_uint8x8_private
      r_,
      a_ = simde_uint8x8_to_private(a),
      b_ = simde_uint8x8_to_private(b);

    #if defined(SIMDE_X86_SSSE3_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      r_.m64 = _mm_shuffle_pi8(a_.m64, _mm_or_si64(b_.m64, _mm_cmpgt_pi8(b_.m64, _mm_set1_pi8(7))));
    #elif defined(SIMDE_RISCV_V_NATIVE)
      vbool8_t mask = __riscv_vmsgeu_vx_u8m1_b8 (b_.sv64, 8, 8);
      r_.sv64 = __riscv_vrgather_vv_u8m1(a_.sv64 , b_.sv64 , 8);
      r_.sv64 = __riscv_vmerge_vxm_u8m1(r_.sv64, 0, mask, 8);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (b_.values[i] < 8) ? a_.values[b_.values[i]] : 0;
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl1_u8
  #define vtbl1_u8(a, b) simde_vtbl1_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vtbl1_s8(simde_int8x8_t a, simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl1_s8(a, b);
  #else
    return simde_vreinterpret_s8_u8(simde_vtbl1_u8(simde_vreinterpret_u8_s8(a), simde_vreinterpret_u8_s8(b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl1_s8
  #define vtbl1_s8(a, b) simde_vtbl1_s8((a), (b))
#endif

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vtbl2_u8(simde_uint8x8x2_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl2_u8(a, b);
  #else
    simde_uint8x8_private
      r_,
      a_[2] = { simde_uint8x8_to_private(a.val[0]), simde_uint8x8_to_private(a.val[1]) },
      b_ = simde_uint8x8_to_private(b);

    #if defined(SIMDE_X86_SSSE3_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i a128 = _mm_set_epi64(a_[1].m64, a_[0].m64);
      __m128i b128 = _mm_set1_epi64(b_.m64);
      __m128i r128 = _mm_shuffle_epi8(a128, _mm_or_si128(b128, _mm_cmpgt_epi8(b128, _mm_set1_epi8(15))));
      r_.m64 = _mm_movepi64_pi64(r128);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1_t t_combine = __riscv_vslideup_vx_u8m1(a_[0].sv64 , a_[1].sv64 , 8 , 16);
      vbool8_t mask = __riscv_vmsgeu_vx_u8m1_b8 (b_.sv64 , 16 , 8);
      vuint8m1_t r_tmp = __riscv_vrgather_vv_u8m1(t_combine , b_.sv64 , 8);
      r_.sv64 = __riscv_vmerge_vxm_u8m1(r_tmp, 0, mask, 8);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (b_.values[i] < 16) ? a_[b_.values[i] / 8].values[b_.values[i] & 7] : 0;
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl2_u8
  #define vtbl2_u8(a, b) simde_vtbl2_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vtbl2_s8(simde_int8x8x2_t a, simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl2_s8(a, b);
  #else
    simde_uint8x8x2_t a_;
    simde_memcpy(&a_, &a, sizeof(a_));
    return simde_vreinterpret_s8_u8(simde_vtbl2_u8(a_, simde_vreinterpret_u8_s8(b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl2_s8
  #define vtbl2_s8(a, b) simde_vtbl2_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vtbl3_u8(simde_uint8x8x3_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl3_u8(a, b);
  #else
    simde_uint8x8_private
      r_,
      a_[3] = { simde_uint8x8_to_private(a.val[0]), simde_uint8x8_to_private(a.val[1]), simde_uint8x8_to_private(a.val[2]) },
      b_ = simde_uint8x8_to_private(b);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i b128 = _mm_set1_epi64(b_.m64);
      b128 = _mm_or_si128(b128, _mm_cmpgt_epi8(b128, _mm_set1_epi8(23)));
      __m128i r128_01 = _mm_shuffle_epi8(_mm_set_epi64(a_[1].m64, a_[0].m64), b128);
      __m128i r128_2  = _mm_shuffle_epi8(_mm_set1_epi64(a_[2].m64), b128);
      __m128i r128 = _mm_blendv_epi8(r128_01, r128_2, _mm_slli_epi32(b128, 3));
      r_.m64 = _mm_movepi64_pi64(r128);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      vuint8m2_t t1 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[0].sv64);
      vuint8m2_t t2 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[1].sv64);
      vuint8m2_t t3 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[2].sv64);
      vuint8m2_t t_combine = __riscv_vslideup_vx_u8m2(t2 , t3 , 8 , 24);
      t_combine = __riscv_vslideup_vx_u8m2(t1 , t_combine , 8 , 24);
      vuint8m2_t idxm2 = __riscv_vlmul_ext_v_u8m1_u8m2(b_.sv64);
      vbool4_t mask = __riscv_vmsgeu_vx_u8m2_b4 (idxm2, 24, 8);
      vuint8m2_t r_tmp = __riscv_vrgather_vv_u8m2(t_combine , idxm2 , 8);
      r_.sv64 = __riscv_vlmul_trunc_v_u8m2_u8m1(__riscv_vmerge_vxm_u8m2(r_tmp, 0, mask, 8));
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (b_.values[i] < 24) ? a_[b_.values[i] / 8].values[b_.values[i] & 7] : 0;
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl3_u8
  #define vtbl3_u8(a, b) simde_vtbl3_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vtbl3_s8(simde_int8x8x3_t a, simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl3_s8(a, b);
  #else
    simde_uint8x8x3_t a_;
    simde_memcpy(&a_, &a, sizeof(a_));
    return simde_vreinterpret_s8_u8(simde_vtbl3_u8(a_, simde_vreinterpret_u8_s8(b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl3_s8
  #define vtbl3_s8(a, b) simde_vtbl3_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vtbl4_u8(simde_uint8x8x4_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl4_u8(a, b);
  #else
    simde_uint8x8_private
      r_,
      a_[4] = { simde_uint8x8_to_private(a.val[0]), simde_uint8x8_to_private(a.val[1]), simde_uint8x8_to_private(a.val[2]), simde_uint8x8_to_private(a.val[3]) },
      b_ = simde_uint8x8_to_private(b);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i b128 = _mm_set1_epi64(b_.m64);
      b128 = _mm_or_si128(b128, _mm_cmpgt_epi8(b128, _mm_set1_epi8(31)));
      __m128i r128_01 = _mm_shuffle_epi8(_mm_set_epi64(a_[1].m64, a_[0].m64), b128);
      __m128i r128_23 = _mm_shuffle_epi8(_mm_set_epi64(a_[3].m64, a_[2].m64), b128);
      __m128i r128 = _mm_blendv_epi8(r128_01, r128_23, _mm_slli_epi32(b128, 3));
      r_.m64 = _mm_movepi64_pi64(r128);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      vuint8m2_t t1 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[0].sv64);
      vuint8m2_t t2 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[1].sv64);
      vuint8m2_t t3 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[2].sv64);
      vuint8m2_t t4 = __riscv_vlmul_ext_v_u8m1_u8m2 (a_[3].sv64);
      vuint8m2_t t_combine = __riscv_vslideup_vx_u8m2(t3 , t4 , 8 , 32);
      t_combine = __riscv_vslideup_vx_u8m2(t2 , t_combine , 8 , 32);
      t_combine = __riscv_vslideup_vx_u8m2(t1 , t_combine , 8 , 32);
      vuint8m2_t idxm2 = __riscv_vlmul_ext_v_u8m1_u8m2(b_.sv64);
      vbool4_t mask = __riscv_vmsgeu_vx_u8m2_b4 (idxm2, 32, 8);
      vuint8m2_t r_tmp = __riscv_vrgather_vv_u8m2(t_combine , idxm2 , 8);
      r_.sv64 = __riscv_vlmul_trunc_v_u8m2_u8m1(__riscv_vmerge_vxm_u8m2(r_tmp, 0, mask, 8));
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (b_.values[i] < 32) ? a_[b_.values[i] / 8].values[b_.values[i] & 7] : 0;
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl4_u8
  #define vtbl4_u8(a, b) simde_vtbl4_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vtbl4_s8(simde_int8x8x4_t a, simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl4_s8(a, b);
  #else
    simde_uint8x8x4_t a_;
    simde_memcpy(&a_, &a, sizeof(a_));
    return simde_vreinterpret_s8_u8(simde_vtbl4_u8(a_, simde_vreinterpret_u8_s8(b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl4_s8
  #define vtbl4_s8(a, b) simde_vtbl4_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vtbl1_p8(simde_poly8x8_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl1_p8(a, b);
  #else
    return simde_vreinterpret_p8_u8(simde_vtbl1_u8(simde_vreinterpret_u8_p8(a), b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl1_p8
  #define vtbl1_p8(a, b) simde_vtbl1_p8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vtbl2_p8(simde_poly8x8x2_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl2_p8(a, b);
  #else
    simde_uint8x8x2_t a_;
    simde_memcpy(&a_, &a, sizeof(a_));
    return simde_vreinterpret_p8_u8(simde_vtbl2_u8(a_, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl2_p8
  #define vtbl2_p8(a, b) simde_vtbl2_p8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vtbl3_p8(simde_poly8x8x3_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl3_p8(a, b);
  #else
    simde_uint8x8x3_t a_;
    simde_memcpy(&a_, &a, sizeof(a_));
    return simde_vreinterpret_p8_u8(simde_vtbl3_u8(a_, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl3_p8
  #define vtbl3_p8(a, b) simde_vtbl3_p8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vtbl4_p8(simde_poly8x8x4_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vtbl4_p8(a, b);
  #else
    simde_uint8x8x4_t a_;
    simde_memcpy(&a_, &a, sizeof(a_));
    return simde_vreinterpret_p8_u8(simde_vtbl4_u8(a_, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vtbl4_p8
  #define vtbl4_p8(a, b) simde_vtbl4_p8((a), (b))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_TBL_H) */
