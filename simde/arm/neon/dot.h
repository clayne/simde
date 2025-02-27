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
 *   2020      Sean Maher <seanptmaher@gmail.com> (Copyright owned by Google, LLC)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_DOT_H)
#define SIMDE_ARM_NEON_DOT_H

#include "types.h"

#include "add.h"
#include "combine.h"
#include "dup_n.h"
#include "get_low.h"
#include "get_high.h"
#include "paddl.h"
#include "movn.h"
#include "mull.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vdot_s32(simde_int32x2_t r, simde_int8x8_t a, simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_DOTPROD)
    return vdot_s32(r, a, b);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return simde_vadd_s32(r, simde_vmovn_s64(simde_vpaddlq_s32(simde_vpaddlq_s16(simde_vmull_s8(a, b)))));
  #else
    simde_int32x2_private r_;
    simde_int8x8_private
      a_ = simde_int8x8_to_private(a),
      b_ = simde_int8x8_to_private(b);
      #if defined(SIMDE_RISCV_V_NATIVE)
        simde_int32x2_private r_tmp = simde_int32x2_to_private(r);
        vint16m2_t vd_low = __riscv_vwmul_vv_i16m2 (a_.sv64, b_.sv64, 8);
        vint16m2_t vd_high = __riscv_vslidedown_vx_i16m2(vd_low, 4, 8);
        vint32m1_t vd = __riscv_vmv_v_x_i32m1(0, 4);
        vint32m1_t vd_low_wide = __riscv_vwcvt_x_x_v_i32m1 (__riscv_vlmul_trunc_v_i16m2_i16mf2(vd_low), 4);
        vint32m1_t rst0 = __riscv_vredsum_vs_i32m1_i32m1(vd_low_wide, vd, 4);
        vint32m1_t vd_high_wide = __riscv_vwcvt_x_x_v_i32m1 (__riscv_vlmul_trunc_v_i16m2_i16mf2(vd_high), 4);
        vint32m1_t rst1 = __riscv_vredsum_vs_i32m1_i32m1(vd_high_wide, vd, 4);
        r_.sv64 = __riscv_vslideup_vx_i32m1(
          __riscv_vadd_vx_i32m1(rst0, r_tmp.values[0], 2),
          __riscv_vadd_vx_i32m1(rst1, r_tmp.values[1], 2),
          1, 2);
        return simde_int32x2_from_private(r_);
      #else
        for (int i = 0 ; i < 2 ; i++) {
          int32_t acc = 0;
          SIMDE_VECTORIZE_REDUCTION(+:acc)
          for (int j = 0 ; j < 4 ; j++) {
            const int idx = j + (i << 2);
            acc += HEDLEY_STATIC_CAST(int32_t, a_.values[idx]) * HEDLEY_STATIC_CAST(int32_t, b_.values[idx]);
          }
          r_.values[i] = acc;
        }
        #endif
      return simde_vadd_s32(r, simde_int32x2_from_private(r_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES) || (defined(SIMDE_ENABLE_NATIVE_ALIASES) && \
  !(defined(SIMDE_ARCH_ARM_DOTPROD)))
  #undef vdot_s32
  #define vdot_s32(r, a, b) simde_vdot_s32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vdot_u32(simde_uint32x2_t r, simde_uint8x8_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_DOTPROD)
    return vdot_u32(r, a, b);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return simde_vadd_u32(r, simde_vmovn_u64(simde_vpaddlq_u32(simde_vpaddlq_u16(simde_vmull_u8(a, b)))));
  #else
    simde_uint32x2_private r_;
    simde_uint8x8_private
      a_ = simde_uint8x8_to_private(a),
      b_ = simde_uint8x8_to_private(b);

      #if defined(SIMDE_RISCV_V_NATIVE)
        simde_uint32x2_private r_tmp = simde_uint32x2_to_private(r);
        vuint16m2_t vd_low = __riscv_vwmulu_vv_u16m2 (a_.sv64, b_.sv64, 8);
        vuint16m2_t vd_high = __riscv_vslidedown_vx_u16m2(vd_low, 4, 8);
        vuint32m1_t vd = __riscv_vmv_v_x_u32m1(0, 4);
        vuint32m1_t vd_low_wide = __riscv_vwcvtu_x_x_v_u32m1 (__riscv_vlmul_trunc_v_u16m2_u16mf2(vd_low), 4);
        vuint32m1_t rst0 = __riscv_vredsum_vs_u32m1_u32m1(vd_low_wide, vd, 4);
        vuint32m1_t vd_high_wide = __riscv_vwcvtu_x_x_v_u32m1 (__riscv_vlmul_trunc_v_u16m2_u16mf2(vd_high), 4);
        vuint32m1_t rst1 = __riscv_vredsum_vs_u32m1_u32m1(vd_high_wide, vd, 4);
        r_.sv64 = __riscv_vslideup_vx_u32m1(
          __riscv_vadd_vx_u32m1(rst0, r_tmp.values[0], 2),
          __riscv_vadd_vx_u32m1(rst1, r_tmp.values[1], 2),
          1, 2);
        return simde_uint32x2_from_private(r_);
      #else
        for (int i = 0 ; i < 2 ; i++) {
          uint32_t acc = 0;
          SIMDE_VECTORIZE_REDUCTION(+:acc)
          for (int j = 0 ; j < 4 ; j++) {
            const int idx = j + (i << 2);
            acc += HEDLEY_STATIC_CAST(uint32_t, a_.values[idx]) * HEDLEY_STATIC_CAST(uint32_t, b_.values[idx]);
          }
          r_.values[i] = acc;
        }
      #endif
    return simde_vadd_u32(r, simde_uint32x2_from_private(r_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES) || (defined(SIMDE_ENABLE_NATIVE_ALIASES) && \
  !(defined(SIMDE_ARCH_ARM_DOTPROD)))
  #undef vdot_u32
  #define vdot_u32(r, a, b) simde_vdot_u32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vdotq_s32(simde_int32x4_t r, simde_int8x16_t a, simde_int8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_DOTPROD)
    return vdotq_s32(r, a, b);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return simde_vaddq_s32(r,
                           simde_vcombine_s32(simde_vmovn_s64(simde_vpaddlq_s32(simde_vpaddlq_s16(simde_vmull_s8(simde_vget_low_s8(a), simde_vget_low_s8(b))))),
                                                              simde_vmovn_s64(simde_vpaddlq_s32(simde_vpaddlq_s16(simde_vmull_s8(simde_vget_high_s8(a), simde_vget_high_s8(b)))))));
  #else
    simde_int32x4_private r_;
    simde_int8x16_private
      a_ = simde_int8x16_to_private(a),
      b_ = simde_int8x16_to_private(b);
      #if defined(SIMDE_RISCV_V_NATIVE)
        simde_int32x4_private r_tmp = simde_int32x4_to_private(r);
        vint16m2_t vd_low = __riscv_vwmul_vv_i16m2 (a_.sv128, b_.sv128, 16);
        vint32m1_t vd = __riscv_vmv_v_x_i32m1(0, 4);
        vint32m1_t rst0 = __riscv_vredsum_vs_i32m1_i32m1(__riscv_vwcvt_x_x_v_i32m1(__riscv_vlmul_trunc_v_i16m2_i16mf2( \
          vd_low), 4), vd, 4);
        vint32m1_t rst1 = __riscv_vredsum_vs_i32m1_i32m1(__riscv_vwcvt_x_x_v_i32m1(__riscv_vlmul_trunc_v_i16m2_i16mf2( \
          __riscv_vslidedown_vx_i16m2(vd_low, 4, 4)), 4), vd, 4);
        vint32m1_t rst2 = __riscv_vredsum_vs_i32m1_i32m1(__riscv_vwcvt_x_x_v_i32m1(__riscv_vlmul_trunc_v_i16m2_i16mf2( \
          __riscv_vslidedown_vx_i16m2(vd_low, 8, 4)), 4), vd, 4);
        vint32m1_t rst3 = __riscv_vredsum_vs_i32m1_i32m1(__riscv_vwcvt_x_x_v_i32m1(__riscv_vlmul_trunc_v_i16m2_i16mf2( \
          __riscv_vslidedown_vx_i16m2(vd_low, 12, 4)), 4), vd, 4);
        vint32m1_t r0 = __riscv_vslideup_vx_i32m1(__riscv_vadd_vx_i32m1(rst0, r_tmp.values[0], 2), __riscv_vadd_vx_i32m1(rst1, r_tmp.values[1], 2), 1, 2);
        vint32m1_t r1 = __riscv_vslideup_vx_i32m1(r0, __riscv_vadd_vx_i32m1(rst2, r_tmp.values[2], 2), 2, 3);
        r_.sv128 = __riscv_vslideup_vx_i32m1(r1, __riscv_vadd_vx_i32m1(rst3, r_tmp.values[3], 2), 3, 4);
       return simde_int32x4_from_private(r_);
    #else
        for (int i = 0 ; i < 4 ; i++) {
          int32_t acc = 0;
          SIMDE_VECTORIZE_REDUCTION(+:acc)
          for (int j = 0 ; j < 4 ; j++) {
            const int idx = j + (i << 2);
            acc += HEDLEY_STATIC_CAST(int32_t, a_.values[idx]) * HEDLEY_STATIC_CAST(int32_t, b_.values[idx]);
          }
          r_.values[i] = acc;
        }
    #endif
    return simde_vaddq_s32(r, simde_int32x4_from_private(r_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES) || (defined(SIMDE_ENABLE_NATIVE_ALIASES) && \
  !(defined(SIMDE_ARCH_ARM_DOTPROD)))
  #undef vdotq_s32
  #define vdotq_s32(r, a, b) simde_vdotq_s32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vdotq_u32(simde_uint32x4_t r, simde_uint8x16_t a, simde_uint8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_DOTPROD)
    return vdotq_u32(r, a, b);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return simde_vaddq_u32(r,
                           simde_vcombine_u32(simde_vmovn_u64(simde_vpaddlq_u32(simde_vpaddlq_u16(simde_vmull_u8(simde_vget_low_u8(a), simde_vget_low_u8(b))))),
                                              simde_vmovn_u64(simde_vpaddlq_u32(simde_vpaddlq_u16(simde_vmull_u8(simde_vget_high_u8(a), simde_vget_high_u8(b)))))));
  #else
    simde_uint32x4_private r_;
    simde_uint8x16_private
      a_ = simde_uint8x16_to_private(a),
      b_ = simde_uint8x16_to_private(b);
      #if defined(SIMDE_RISCV_V_NATIVE)
        simde_uint32x4_private r_tmp = simde_uint32x4_to_private(r);
        vuint16m2_t vd_low = __riscv_vwmulu_vv_u16m2 (a_.sv128, b_.sv128, 16);
        vuint32m1_t vd = __riscv_vmv_v_x_u32m1(0, 4);
        vuint32m1_t rst0 = __riscv_vredsum_vs_u32m1_u32m1(__riscv_vwcvtu_x_x_v_u32m1(__riscv_vlmul_trunc_v_u16m2_u16mf2( \
          vd_low), 4), vd, 4);
        vuint32m1_t rst1 = __riscv_vredsum_vs_u32m1_u32m1(__riscv_vwcvtu_x_x_v_u32m1(__riscv_vlmul_trunc_v_u16m2_u16mf2( \
          __riscv_vslidedown_vx_u16m2(vd_low, 4, 4)), 4), vd, 4);
        vuint32m1_t rst2 = __riscv_vredsum_vs_u32m1_u32m1(__riscv_vwcvtu_x_x_v_u32m1(__riscv_vlmul_trunc_v_u16m2_u16mf2( \
          __riscv_vslidedown_vx_u16m2(vd_low, 8, 4)), 4), vd, 4);
        vuint32m1_t rst3 = __riscv_vredsum_vs_u32m1_u32m1(__riscv_vwcvtu_x_x_v_u32m1(__riscv_vlmul_trunc_v_u16m2_u16mf2( \
          __riscv_vslidedown_vx_u16m2(vd_low, 12, 4)), 4), vd, 4);
        vuint32m1_t r0 = __riscv_vslideup_vx_u32m1(__riscv_vadd_vx_u32m1(rst0, r_tmp.values[0], 2), __riscv_vadd_vx_u32m1(rst1, r_tmp.values[1], 2), 1, 2);
        vuint32m1_t r1 = __riscv_vslideup_vx_u32m1(r0, __riscv_vadd_vx_u32m1(rst2, r_tmp.values[2], 2), 2, 3);
        r_.sv128 = __riscv_vslideup_vx_u32m1(r1, __riscv_vadd_vx_u32m1(rst3, r_tmp.values[3], 2), 3, 4);
        return simde_uint32x4_from_private(r_);
    #else
      for (int i = 0 ; i < 4 ; i++) {
        uint32_t acc = 0;
        SIMDE_VECTORIZE_REDUCTION(+:acc)
        for (int j = 0 ; j < 4 ; j++) {
          const int idx = j + (i << 2);
          acc += HEDLEY_STATIC_CAST(uint32_t, a_.values[idx]) * HEDLEY_STATIC_CAST(uint32_t, b_.values[idx]);
        }
        r_.values[i] = acc;
      }
    #endif
    return simde_vaddq_u32(r, simde_uint32x4_from_private(r_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES) || (defined(SIMDE_ENABLE_NATIVE_ALIASES) && \
  !(defined(SIMDE_ARCH_ARM_DOTPROD)))
  #undef vdotq_u32
  #define vdotq_u32(r, a, b) simde_vdotq_u32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vbfdot_f32(simde_float32x2_t r, simde_bfloat16x4_t a, simde_bfloat16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && \
      defined(SIMDE_ARM_NEON_BF16)
    return vbfdot_f32(r, a, b);
  #else
    simde_float32x2_private r_ = simde_float32x2_to_private(r);
    simde_bfloat16x4_private
      a_ = simde_bfloat16x4_to_private(a),
      b_ = simde_bfloat16x4_to_private(b);

    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      simde_float32_t elt1_a = simde_bfloat16_to_float32(a_.values[2 * i + 0]);
      simde_float32_t elt1_b = simde_bfloat16_to_float32(a_.values[2 * i + 1]);
      simde_float32_t elt2_a = simde_bfloat16_to_float32(b_.values[2 * i + 0]);
      simde_float32_t elt2_b = simde_bfloat16_to_float32(b_.values[2 * i + 1]);
      r_.values[i] = r_.values[i] + elt1_a * elt2_a + elt1_b * elt2_b;
    }
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES) || (defined(SIMDE_ENABLE_NATIVE_ALIASES) && \
  !(defined(SIMDE_ARM_NEON_BF16)))
  #undef vbfdot_f32
  #define vbfdot_f32(r, a, b) simde_vbfdot_f32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfdotq_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_DOTPROD) && \
      defined(SIMDE_ARM_NEON_BF16)
    return vbfdotq_f32(r, a, b);
  #else
    simde_float32x4_private r_ = simde_float32x4_to_private(r);
    simde_bfloat16x8_private
      a_ = simde_bfloat16x8_to_private(a),
      b_ = simde_bfloat16x8_to_private(b);

    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      simde_float32_t elt1_a = simde_bfloat16_to_float32(a_.values[2 * i + 0]);
      simde_float32_t elt1_b = simde_bfloat16_to_float32(a_.values[2 * i + 1]);
      simde_float32_t elt2_a = simde_bfloat16_to_float32(b_.values[2 * i + 0]);
      simde_float32_t elt2_b = simde_bfloat16_to_float32(b_.values[2 * i + 1]);
      r_.values[i] = r_.values[i] + elt1_a * elt2_a + elt1_b * elt2_b;
    }
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES) || (defined(SIMDE_ENABLE_NATIVE_ALIASES) && \
  !(defined(SIMDE_ARCH_ARM_DOTPROD) && \
      defined(SIMDE_ARM_NEON_BF16)))
  #undef vbfdotq_f32
  #define vbfdotq_f32(r, a, b) simde_vbfdotq_f32((r), (a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_DOT_H) */
