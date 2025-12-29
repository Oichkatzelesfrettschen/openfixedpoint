# Complete SIMD Intrinsic Catalog for Fixed-Point

## x86 SSE/SSE2/SSE4.1

### 128-bit Integer Operations

#### Load/Store
```c
// Aligned load (16-byte alignment required)
__m128i _mm_load_si128(__m128i const* p);
__m128i _mm_loadu_si128(__m128i const* p);  // Unaligned

// Store
void _mm_store_si128(__m128i* p, __m128i a);
void _mm_storeu_si128(__m128i* p, __m128i a);  // Unaligned

// Streaming store (non-temporal, bypasses cache)
void _mm_stream_si128(__m128i* p, __m128i a);
```

#### Addition (8/16/32/64-bit elements)
```c
// Wrapping add
__m128i _mm_add_epi8(__m128i a, __m128i b);   // 16 × 8-bit
__m128i _mm_add_epi16(__m128i a, __m128i b);  // 8 × 16-bit
__m128i _mm_add_epi32(__m128i a, __m128i b);  // 4 × 32-bit
__m128i _mm_add_epi64(__m128i a, __m128i b);  // 2 × 64-bit

// Saturating add (signed)
__m128i _mm_adds_epi8(__m128i a, __m128i b);  // 16 × Q7
__m128i _mm_adds_epi16(__m128i a, __m128i b); // 8 × Q15

// Saturating add (unsigned)
__m128i _mm_adds_epu8(__m128i a, __m128i b);  // 16 × UQ8
__m128i _mm_adds_epu16(__m128i a, __m128i b); // 8 × UQ16
```

#### Subtraction
```c
// Wrapping subtract
__m128i _mm_sub_epi8(__m128i a, __m128i b);
__m128i _mm_sub_epi16(__m128i a, __m128i b);
__m128i _mm_sub_epi32(__m128i a, __m128i b);
__m128i _mm_sub_epi64(__m128i a, __m128i b);

// Saturating subtract (signed)
__m128i _mm_subs_epi8(__m128i a, __m128i b);
__m128i _mm_subs_epi16(__m128i a, __m128i b);

// Saturating subtract (unsigned)
__m128i _mm_subs_epu8(__m128i a, __m128i b);
__m128i _mm_subs_epu16(__m128i a, __m128i b);
```

#### Multiplication
```c
// 16-bit multiply: low 16 bits
__m128i _mm_mullo_epi16(__m128i a, __m128i b);

// 16-bit multiply: high 16 bits (signed)
__m128i _mm_mulhi_epi16(__m128i a, __m128i b);  // Essential for Q15

// 16-bit multiply: high 16 bits (unsigned)
__m128i _mm_mulhi_epu16(__m128i a, __m128i b);

// 32-bit multiply: low 32 bits (SSE4.1)
__m128i _mm_mullo_epi32(__m128i a, __m128i b);

// Packed multiply-add: a[i]*b[i] + a[i+1]*b[i+1] → 32-bit
__m128i _mm_madd_epi16(__m128i a, __m128i b);  // Sum adjacent products

// Multiply with rounding and scaling (SSSE3) - Q15 multiply!
__m128i _mm_mulhrs_epi16(__m128i a, __m128i b);  // (a*b + 0x4000) >> 15
```

#### Shifts
```c
// Logical left shift (zeros fill)
__m128i _mm_slli_epi16(__m128i a, int imm);
__m128i _mm_slli_epi32(__m128i a, int imm);
__m128i _mm_slli_epi64(__m128i a, int imm);

// Logical right shift
__m128i _mm_srli_epi16(__m128i a, int imm);
__m128i _mm_srli_epi32(__m128i a, int imm);
__m128i _mm_srli_epi64(__m128i a, int imm);

// Arithmetic right shift (sign extends)
__m128i _mm_srai_epi16(__m128i a, int imm);
__m128i _mm_srai_epi32(__m128i a, int imm);
// Note: No _mm_srai_epi64 until AVX-512!

// Variable shift (SSE4.1) - each element shifted by different amount
__m128i _mm_sllv_epi32(__m128i a, __m128i count);  // AVX2
__m128i _mm_srlv_epi32(__m128i a, __m128i count);  // AVX2
```

#### Comparison
```c
// Returns 0xFF...F for true, 0 for false
__m128i _mm_cmpeq_epi8(__m128i a, __m128i b);
__m128i _mm_cmpeq_epi16(__m128i a, __m128i b);
__m128i _mm_cmpeq_epi32(__m128i a, __m128i b);

__m128i _mm_cmpgt_epi8(__m128i a, __m128i b);   // Signed greater than
__m128i _mm_cmpgt_epi16(__m128i a, __m128i b);
__m128i _mm_cmpgt_epi32(__m128i a, __m128i b);

__m128i _mm_cmplt_epi8(__m128i a, __m128i b);   // Signed less than
__m128i _mm_cmplt_epi16(__m128i a, __m128i b);
__m128i _mm_cmplt_epi32(__m128i a, __m128i b);
```

#### Min/Max (SSE4.1 for all widths)
```c
// Signed min/max
__m128i _mm_min_epi8(__m128i a, __m128i b);   // SSE4.1
__m128i _mm_max_epi8(__m128i a, __m128i b);   // SSE4.1
__m128i _mm_min_epi16(__m128i a, __m128i b);  // SSE2
__m128i _mm_max_epi16(__m128i a, __m128i b);  // SSE2
__m128i _mm_min_epi32(__m128i a, __m128i b);  // SSE4.1
__m128i _mm_max_epi32(__m128i a, __m128i b);  // SSE4.1

// Unsigned min/max
__m128i _mm_min_epu8(__m128i a, __m128i b);
__m128i _mm_max_epu8(__m128i a, __m128i b);
__m128i _mm_min_epu16(__m128i a, __m128i b);  // SSE4.1
__m128i _mm_max_epu16(__m128i a, __m128i b);  // SSE4.1
__m128i _mm_min_epu32(__m128i a, __m128i b);  // SSE4.1
__m128i _mm_max_epu32(__m128i a, __m128i b);  // SSE4.1
```

#### Absolute Value (SSSE3)
```c
__m128i _mm_abs_epi8(__m128i a);
__m128i _mm_abs_epi16(__m128i a);
__m128i _mm_abs_epi32(__m128i a);
```

#### Horizontal Operations (SSSE3)
```c
// Horizontal add (sum adjacent pairs)
__m128i _mm_hadd_epi16(__m128i a, __m128i b);
__m128i _mm_hadd_epi32(__m128i a, __m128i b);

// Horizontal add with saturation
__m128i _mm_hadds_epi16(__m128i a, __m128i b);

// Horizontal subtract
__m128i _mm_hsub_epi16(__m128i a, __m128i b);
__m128i _mm_hsub_epi32(__m128i a, __m128i b);
__m128i _mm_hsubs_epi16(__m128i a, __m128i b);
```

#### Pack and Unpack
```c
// Pack with saturation (signed to signed)
__m128i _mm_packs_epi16(__m128i a, __m128i b);  // 16-bit → 8-bit
__m128i _mm_packs_epi32(__m128i a, __m128i b);  // 32-bit → 16-bit

// Pack with saturation (signed to unsigned)
__m128i _mm_packus_epi16(__m128i a, __m128i b); // 16-bit → unsigned 8-bit
__m128i _mm_packus_epi32(__m128i a, __m128i b); // SSE4.1

// Unpack and interleave
__m128i _mm_unpacklo_epi8(__m128i a, __m128i b);
__m128i _mm_unpackhi_epi8(__m128i a, __m128i b);
__m128i _mm_unpacklo_epi16(__m128i a, __m128i b);
__m128i _mm_unpackhi_epi16(__m128i a, __m128i b);
__m128i _mm_unpacklo_epi32(__m128i a, __m128i b);
__m128i _mm_unpackhi_epi32(__m128i a, __m128i b);
__m128i _mm_unpacklo_epi64(__m128i a, __m128i b);
__m128i _mm_unpackhi_epi64(__m128i a, __m128i b);
```

#### Sign Extension (SSE4.1)
```c
__m128i _mm_cvtepi8_epi16(__m128i a);   // 8 × 8-bit → 8 × 16-bit
__m128i _mm_cvtepi8_epi32(__m128i a);   // 4 × 8-bit → 4 × 32-bit
__m128i _mm_cvtepi8_epi64(__m128i a);   // 2 × 8-bit → 2 × 64-bit
__m128i _mm_cvtepi16_epi32(__m128i a);  // 4 × 16-bit → 4 × 32-bit
__m128i _mm_cvtepi16_epi64(__m128i a);  // 2 × 16-bit → 2 × 64-bit
__m128i _mm_cvtepi32_epi64(__m128i a);  // 2 × 32-bit → 2 × 64-bit

// Zero extension
__m128i _mm_cvtepu8_epi16(__m128i a);
__m128i _mm_cvtepu8_epi32(__m128i a);
__m128i _mm_cvtepu8_epi64(__m128i a);
__m128i _mm_cvtepu16_epi32(__m128i a);
__m128i _mm_cvtepu16_epi64(__m128i a);
__m128i _mm_cvtepu32_epi64(__m128i a);
```

---

## AVX2 (256-bit)

### All SSE operations extended to 256-bit
```c
// Replace _mm_ with _mm256_, __m128i with __m256i
__m256i _mm256_add_epi16(__m256i a, __m256i b);  // 16 × 16-bit
__m256i _mm256_adds_epi16(__m256i a, __m256i b); // Saturating
__m256i _mm256_mulhi_epi16(__m256i a, __m256i b);
__m256i _mm256_mulhrs_epi16(__m256i a, __m256i b); // Q15 multiply!
// ... etc
```

### Cross-Lane Operations (AVX2 specific)
```c
// Permute across 128-bit lanes
__m256i _mm256_permute2x128_si256(__m256i a, __m256i b, int imm);
__m256i _mm256_permutevar8x32_epi32(__m256i a, __m256i idx);

// Broadcast
__m256i _mm256_broadcastb_epi8(__m128i a);   // Broadcast byte
__m256i _mm256_broadcastw_epi16(__m128i a);  // Broadcast word
__m256i _mm256_broadcastd_epi32(__m128i a);  // Broadcast dword
__m256i _mm256_broadcastq_epi64(__m128i a);  // Broadcast qword
```

---

## AVX-512BW/DQ/VBMI

### 512-bit Integer Operations
```c
// Basic arithmetic
__m512i _mm512_add_epi8(__m512i a, __m512i b);   // 64 × 8-bit
__m512i _mm512_add_epi16(__m512i a, __m512i b);  // 32 × 16-bit
__m512i _mm512_add_epi32(__m512i a, __m512i b);  // 16 × 32-bit
__m512i _mm512_add_epi64(__m512i a, __m512i b);  // 8 × 64-bit

// Saturating (AVX-512BW)
__m512i _mm512_adds_epi8(__m512i a, __m512i b);
__m512i _mm512_adds_epi16(__m512i a, __m512i b);
__m512i _mm512_adds_epu8(__m512i a, __m512i b);
__m512i _mm512_adds_epu16(__m512i a, __m512i b);

// Multiplication
__m512i _mm512_mullo_epi16(__m512i a, __m512i b);
__m512i _mm512_mulhi_epi16(__m512i a, __m512i b);
__m512i _mm512_mulhrs_epi16(__m512i a, __m512i b); // Q15!
__m512i _mm512_mullo_epi32(__m512i a, __m512i b);
__m512i _mm512_mullo_epi64(__m512i a, __m512i b);  // AVX-512DQ
```

### Mask Operations (AVX-512 innovation)
```c
// Masks are now first-class types
__mmask64  // 64-bit mask for 8-bit elements
__mmask32  // 32-bit mask for 16-bit elements
__mmask16  // 16-bit mask for 32-bit elements
__mmask8   // 8-bit mask for 64-bit elements

// Masked operations
__m512i _mm512_mask_add_epi16(__m512i src, __mmask32 k, __m512i a, __m512i b);
__m512i _mm512_maskz_add_epi16(__mmask32 k, __m512i a, __m512i b);  // Zero-masked

// Comparison returns mask
__mmask32 _mm512_cmpeq_epi16_mask(__m512i a, __m512i b);
__mmask32 _mm512_cmpgt_epi16_mask(__m512i a, __m512i b);
__mmask32 _mm512_cmplt_epi16_mask(__m512i a, __m512i b);

// Full comparison predicate
__mmask32 _mm512_cmp_epi16_mask(__m512i a, __m512i b, int imm);
// imm: _MM_CMPINT_EQ, _MM_CMPINT_LT, _MM_CMPINT_LE, _MM_CMPINT_NE, etc.
```

### 64-bit Arithmetic Shift (AVX-512F)
```c
// Finally! 64-bit arithmetic right shift
__m512i _mm512_srai_epi64(__m512i a, int imm);
__m512i _mm512_srav_epi64(__m512i a, __m512i count);  // Variable shift
```

### Absolute Value (AVX-512BW)
```c
__m512i _mm512_abs_epi8(__m512i a);
__m512i _mm512_abs_epi16(__m512i a);
__m512i _mm512_abs_epi32(__m512i a);
__m512i _mm512_abs_epi64(__m512i a);
```

---

## ARM NEON (128-bit)

### Type Naming Convention
```
v{op}[q]_{type}
q = 128-bit (quadword), omit for 64-bit
type: s8/u8/s16/u16/s32/u32/s64/u64
```

### Vector Types
```c
int8x8_t    // 8 × 8-bit signed (64-bit total)
int8x16_t   // 16 × 8-bit signed (128-bit)
int16x4_t   // 4 × 16-bit signed (64-bit)
int16x8_t   // 8 × 16-bit signed (128-bit)
int32x2_t   // 2 × 32-bit signed (64-bit)
int32x4_t   // 4 × 32-bit signed (128-bit)
int64x1_t   // 1 × 64-bit signed (64-bit)
int64x2_t   // 2 × 64-bit signed (128-bit)
// Similarly for uint8x8_t, uint16x4_t, etc.
```

### Load/Store
```c
// Load
int16x8_t vld1q_s16(const int16_t* ptr);
int32x4_t vld1q_s32(const int32_t* ptr);

// Load multiple (interleaved)
int16x8x2_t vld2q_s16(const int16_t* ptr);  // 2-way interleave
int16x8x3_t vld3q_s16(const int16_t* ptr);  // 3-way interleave
int16x8x4_t vld4q_s16(const int16_t* ptr);  // 4-way interleave

// Store
void vst1q_s16(int16_t* ptr, int16x8_t val);

// Store lane
void vst1q_lane_s16(int16_t* ptr, int16x8_t val, int lane);
```

### Addition
```c
// Wrapping add
int16x8_t vaddq_s16(int16x8_t a, int16x8_t b);
int32x4_t vaddq_s32(int32x4_t a, int32x4_t b);

// Saturating add
int16x8_t vqaddq_s16(int16x8_t a, int16x8_t b);  // Q15 safe
int32x4_t vqaddq_s32(int32x4_t a, int32x4_t b);  // Q31 safe

// Widening add (8-bit → 16-bit, 16-bit → 32-bit)
int16x8_t vaddl_s8(int8x8_t a, int8x8_t b);     // 8×8 → 8×16
int32x4_t vaddl_s16(int16x4_t a, int16x4_t b);  // 4×16 → 4×32
int64x2_t vaddl_s32(int32x2_t a, int32x2_t b);  // 2×32 → 2×64

// High half widening add
int16x8_t vaddl_high_s8(int8x16_t a, int8x16_t b);  // High 8 elements

// Halving add (result = (a + b) >> 1, no overflow)
int16x8_t vhaddq_s16(int16x8_t a, int16x8_t b);

// Rounding halving add
int16x8_t vrhaddq_s16(int16x8_t a, int16x8_t b);
```

### Subtraction
```c
// Wrapping subtract
int16x8_t vsubq_s16(int16x8_t a, int16x8_t b);

// Saturating subtract
int16x8_t vqsubq_s16(int16x8_t a, int16x8_t b);

// Widening subtract
int16x8_t vsubl_s8(int8x8_t a, int8x8_t b);
int32x4_t vsubl_s16(int16x4_t a, int16x4_t b);

// Halving subtract
int16x8_t vhsubq_s16(int16x8_t a, int16x8_t b);

// Absolute difference
int16x8_t vabdq_s16(int16x8_t a, int16x8_t b);  // |a - b|

// Absolute difference and accumulate
int16x8_t vabaq_s16(int16x8_t acc, int16x8_t a, int16x8_t b);  // acc + |a-b|
```

### Multiplication
```c
// Low multiply (keep low bits)
int16x8_t vmulq_s16(int16x8_t a, int16x8_t b);

// Widening multiply
int32x4_t vmull_s16(int16x4_t a, int16x4_t b);      // Low half
int32x4_t vmull_high_s16(int16x8_t a, int16x8_t b); // High half

// Saturating doubling high multiply (Q15 operation!)
int16x8_t vqdmulhq_s16(int16x8_t a, int16x8_t b);  // (a*b*2) >> 16

// Saturating rounding doubling high multiply (better Q15!)
int16x8_t vqrdmulhq_s16(int16x8_t a, int16x8_t b);  // round((a*b*2) >> 16)

// Multiply-accumulate
int16x8_t vmlaq_s16(int16x8_t acc, int16x8_t a, int16x8_t b);  // acc + a*b
int16x8_t vmlsq_s16(int16x8_t acc, int16x8_t a, int16x8_t b);  // acc - a*b

// Widening multiply-accumulate
int32x4_t vmlal_s16(int32x4_t acc, int16x4_t a, int16x4_t b);

// Saturating doubling multiply-accumulate (for Q15/Q31)
int32x4_t vqdmlal_s16(int32x4_t acc, int16x4_t a, int16x4_t b);
```

### Multiply by Scalar/Lane
```c
// Multiply by scalar
int16x8_t vmulq_n_s16(int16x8_t a, int16_t b);

// Multiply by lane
int16x8_t vmulq_lane_s16(int16x8_t a, int16x4_t b, int lane);
int16x8_t vmulq_laneq_s16(int16x8_t a, int16x8_t b, int lane);

// Saturating doubling high multiply by lane
int16x8_t vqdmulhq_lane_s16(int16x8_t a, int16x4_t b, int lane);
```

### Shifts
```c
// Shift left (immediate)
int16x8_t vshlq_n_s16(int16x8_t a, int n);

// Shift right (immediate)
int16x8_t vshrq_n_s16(int16x8_t a, int n);  // Arithmetic

// Rounding shift right
int16x8_t vrshrq_n_s16(int16x8_t a, int n);

// Saturating shift left
int16x8_t vqshlq_s16(int16x8_t a, int16x8_t shift);

// Shift right and accumulate
int16x8_t vsraq_n_s16(int16x8_t acc, int16x8_t a, int n);

// Shift right and narrow
int8x8_t vshrn_n_s16(int16x8_t a, int n);  // 16-bit → 8-bit

// Rounding shift right and narrow
int8x8_t vrshrn_n_s16(int16x8_t a, int n);

// Saturating shift right and narrow
int8x8_t vqshrn_n_s16(int16x8_t a, int n);

// Saturating rounding shift right and narrow (all features!)
int8x8_t vqrshrn_n_s16(int16x8_t a, int n);
```

### Comparison
```c
// Returns all-ones for true, all-zeros for false
uint16x8_t vceqq_s16(int16x8_t a, int16x8_t b);  // a == b
uint16x8_t vcgeq_s16(int16x8_t a, int16x8_t b);  // a >= b
uint16x8_t vcgtq_s16(int16x8_t a, int16x8_t b);  // a > b
uint16x8_t vcleq_s16(int16x8_t a, int16x8_t b);  // a <= b
uint16x8_t vcltq_s16(int16x8_t a, int16x8_t b);  // a < b
```

### Min/Max
```c
int16x8_t vminq_s16(int16x8_t a, int16x8_t b);
int16x8_t vmaxq_s16(int16x8_t a, int16x8_t b);

// Pairwise min/max (across adjacent elements)
int16x4_t vpmin_s16(int16x4_t a, int16x4_t b);
int16x4_t vpmax_s16(int16x4_t a, int16x4_t b);

// Reduce to single value (ARMv8.2+)
int16_t vminvq_s16(int16x8_t a);  // Min of all elements
int16_t vmaxvq_s16(int16x8_t a);  // Max of all elements
int32_t vaddvq_s32(int32x4_t a);  // Sum of all elements
```

### Absolute Value
```c
int16x8_t vabsq_s16(int16x8_t a);
int16x8_t vqabsq_s16(int16x8_t a);  // Saturating (handles MIN)

// Negate
int16x8_t vnegq_s16(int16x8_t a);
int16x8_t vqnegq_s16(int16x8_t a);  // Saturating
```

### Bitwise Operations
```c
int16x8_t vandq_s16(int16x8_t a, int16x8_t b);   // AND
int16x8_t vorrq_s16(int16x8_t a, int16x8_t b);   // OR
int16x8_t veorq_s16(int16x8_t a, int16x8_t b);   // XOR
int16x8_t vbicq_s16(int16x8_t a, int16x8_t b);   // a & ~b
int16x8_t vornq_s16(int16x8_t a, int16x8_t b);   // a | ~b
int16x8_t vmvnq_s16(int16x8_t a);                // NOT

// Bit select (mask ? a : b)
int16x8_t vbslq_s16(uint16x8_t mask, int16x8_t a, int16x8_t b);
```

### Type Reinterpretation
```c
// No computation, just reinterpret bits
int32x4_t vreinterpretq_s32_s16(int16x8_t a);
int16x8_t vreinterpretq_s16_s32(int32x4_t a);
uint16x8_t vreinterpretq_u16_s16(int16x8_t a);
```

---

## ARM SVE/SVE2

### Scalable Vector Types
```c
svint8_t   // VL/8 × 8-bit signed
svint16_t  // VL/16 × 16-bit signed
svint32_t  // VL/32 × 32-bit signed
svint64_t  // VL/64 × 64-bit signed
svbool_t   // Predicate (VL/8 bits)
```

### Predication (SVE's Innovation)
```c
// Active lane predicate for current vector length
svbool_t svptrue_b8(void);   // All 8-bit lanes active
svbool_t svptrue_b16(void);  // All 16-bit lanes active
svbool_t svptrue_b32(void);  // All 32-bit lanes active
svbool_t svptrue_b64(void);  // All 64-bit lanes active

// Loop control predicate
svbool_t svwhilelt_b16(int64_t op1, int64_t op2);  // while (i < n)
svbool_t svwhilele_b16(int64_t op1, int64_t op2);  // while (i <= n)

// Count active lanes
uint64_t svcntp_b16(svbool_t pg, svbool_t op);
uint64_t svcntw(void);  // Words per vector
```

### Arithmetic (All Predicated)
```c
// Addition
svint16_t svadd_s16_m(svbool_t pg, svint16_t op1, svint16_t op2);  // Merge
svint16_t svadd_s16_x(svbool_t pg, svint16_t op1, svint16_t op2);  // Don't care
svint16_t svadd_s16_z(svbool_t pg, svint16_t op1, svint16_t op2);  // Zero

// Saturating add
svint16_t svqadd_s16(svint16_t op1, svint16_t op2);

// Halving add
svint16_t svhadd_s16_m(svbool_t pg, svint16_t op1, svint16_t op2);
svint16_t svrhadd_s16_m(svbool_t pg, svint16_t op1, svint16_t op2);  // Rounding

// Multiplication
svint16_t svmul_s16_m(svbool_t pg, svint16_t op1, svint16_t op2);

// Multiply high (SVE2)
svint16_t svmulh_s16_m(svbool_t pg, svint16_t op1, svint16_t op2);

// Saturating rounding doubling multiply high (Q15!)
svint16_t svqrdmulh_s16(svint16_t op1, svint16_t op2);

// Multiply-accumulate
svint16_t svmla_s16_m(svbool_t pg, svint16_t op1, svint16_t op2, svint16_t op3);
// op1 + op2 * op3

// Saturating doubling multiply-accumulate high (Q15 MAC)
svint16_t svqrdmlah_s16(svint16_t op1, svint16_t op2, svint16_t op3);
```

### SVE2 Specific (Enhanced DSP)
```c
// Complex multiply-accumulate
svint16_t svcmla_s16_m(svbool_t pg, svint16_t op1, svint16_t op2,
                       svint16_t op3, uint64_t rot);  // rot: 0, 90, 180, 270

// Polynomial multiply
svint8_t svpmul_n_u8(svuint8_t op1, uint8_t op2);

// Bit manipulation
svint16_t svbdep_s16(svint16_t op1, svint16_t op2);  // Bit deposit
svint16_t svbext_s16(svint16_t op1, svint16_t op2);  // Bit extract

// Match detection
svbool_t svmatch_s16(svbool_t pg, svint16_t op1, svint16_t op2);
```

### Load/Store (Contiguous and Gather/Scatter)
```c
// Contiguous load with predicate
svint16_t svld1_s16(svbool_t pg, const int16_t* base);

// First-faulting load (stops at first fault)
svint16_t svldff1_s16(svbool_t pg, const int16_t* base);

// Gather load (indexed)
svint32_t svld1_gather_index_s32(svbool_t pg, const int32_t* base, svint32_t idx);

// Contiguous store
void svst1_s16(svbool_t pg, int16_t* base, svint16_t data);

// Scatter store
void svst1_scatter_index_s32(svbool_t pg, int32_t* base, svint32_t idx, svint32_t data);
```

---

## RISC-V Vector Extension (RVV 1.0)

### Configuration
```c
// Set vector length
size_t vsetvl_e16m1(size_t avl);  // 16-bit elements, m1 grouping
size_t vsetvl_e32m4(size_t avl);  // 32-bit elements, m4 grouping

// Get max vector length
size_t vsetvlmax_e16m1(void);
```

### Vector Types
```c
vint8m1_t   // 8-bit, LMUL=1
vint16m1_t  // 16-bit, LMUL=1
vint16m2_t  // 16-bit, LMUL=2 (double length)
vint16m4_t  // 16-bit, LMUL=4 (quad length)
vint32m1_t  // 32-bit, LMUL=1
vbool16_t   // Mask for 16-bit elements
```

### Arithmetic
```c
// Addition
vint16m1_t vadd_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vint16m1_t vadd_vx_i16m1(vint16m1_t vs2, int16_t rs1, size_t vl);

// Saturating add
vint16m1_t vsadd_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vint16m1_t vsaddu_vv_u16m1(vuint16m1_t vs2, vuint16m1_t vs1, size_t vl);

// Averaging add (halving add)
vint16m1_t vaadd_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);

// Multiplication
vint16m1_t vmul_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);

// Widening multiply
vint32m2_t vwmul_vv_i32m2(vint16m1_t vs2, vint16m1_t vs1, size_t vl);

// Multiply high
vint16m1_t vmulh_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);  // Signed
vuint16m1_t vmulhu_vv_u16m1(vuint16m1_t vs2, vuint16m1_t vs1, size_t vl);  // Unsigned

// Multiply-accumulate
vint16m1_t vmacc_vv_i16m1(vint16m1_t vd, vint16m1_t vs1, vint16m1_t vs2, size_t vl);
// vd = vd + vs1 * vs2

// Widening multiply-add
vint32m2_t vwmacc_vv_i32m2(vint32m2_t vd, vint16m1_t vs1, vint16m1_t vs2, size_t vl);
```

### Fixed-Point CSRs (RISC-V V extension feature)
```c
// vxsat: Saturation flag (read/write)
// When saturation occurs, vxsat is set to 1

// vxrm: Rounding mode (2 bits)
// 0: round-to-nearest-up
// 1: round-to-nearest-even
// 2: round-down (truncate)
// 3: round-to-odd

// Saturating operations with rounding
vint16m1_t vssra_vx_i16m1(vint16m1_t vs2, uint8_t rs1, size_t vl);  // Shift right + round
vint16m1_t vnclip_wv_i16m1(vint32m2_t vs2, vuint16m1_t shift, size_t vl);  // Narrowing clip
```

### Shifts
```c
// Shift left
vint16m1_t vsll_vv_i16m1(vint16m1_t vs2, vuint16m1_t vs1, size_t vl);
vint16m1_t vsll_vx_i16m1(vint16m1_t vs2, size_t rs1, size_t vl);

// Arithmetic shift right
vint16m1_t vsra_vv_i16m1(vint16m1_t vs2, vuint16m1_t vs1, size_t vl);
vint16m1_t vsra_vx_i16m1(vint16m1_t vs2, size_t rs1, size_t vl);

// Logical shift right
vuint16m1_t vsrl_vv_u16m1(vuint16m1_t vs2, vuint16m1_t vs1, size_t vl);

// Scaling shift right with rounding (uses vxrm)
vint16m1_t vssra_vx_i16m1(vint16m1_t vs2, size_t rs1, size_t vl);

// Narrowing shift right (saturating, uses vxrm)
vint8m1_t vnclip_wx_i8m1(vint16m2_t vs2, size_t rs1, size_t vl);
```

### Comparison
```c
// Set mask
vbool16_t vmseq_vv_i16m1_b16(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vbool16_t vmsne_vv_i16m1_b16(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vbool16_t vmslt_vv_i16m1_b16(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vbool16_t vmsle_vv_i16m1_b16(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vbool16_t vmsgt_vv_i16m1_b16(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vbool16_t vmsge_vv_i16m1_b16(vint16m1_t vs2, vint16m1_t vs1, size_t vl);

// Min/Max
vint16m1_t vmin_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
vint16m1_t vmax_vv_i16m1(vint16m1_t vs2, vint16m1_t vs1, size_t vl);
```

### Reduction Operations
```c
// Sum reduction
vint32m1_t vredsum_vs_i16m1_i32m1(vint32m1_t vd, vint16m1_t vs2,
                                   vint32m1_t vs1, size_t vl);
// vd[0] = vs1[0] + sum(vs2[0:vl])

// Widening sum reduction
vint32m1_t vwredsum_vs_i16m1_i32m1(vint32m1_t vd, vint16m1_t vs2,
                                    vint32m1_t vs1, size_t vl);

// Min/Max reduction
vint16m1_t vredmin_vs_i16m1_i16m1(vint16m1_t vd, vint16m1_t vs2,
                                   vint16m1_t vs1, size_t vl);
vint16m1_t vredmax_vs_i16m1_i16m1(vint16m1_t vd, vint16m1_t vs2,
                                   vint16m1_t vs1, size_t vl);
```

---

## Q15 Multiply Implementation Comparison

### Best Practice Per Platform

| Platform | Intrinsic | Operation |
|----------|-----------|-----------|
| SSE3+ | `_mm_mulhrs_epi16` | (a*b + 0x4000) >> 15 |
| AVX2 | `_mm256_mulhrs_epi16` | Same, 16 elements |
| AVX-512 | `_mm512_mulhrs_epi16` | Same, 32 elements |
| NEON | `vqrdmulhq_s16` | round((a*b*2) >> 16) |
| SVE2 | `svqrdmulh_s16` | Same, VLA |
| RISC-V V | `vmulh` + `vxrm=1` | High bits with rounding |

### Q31 Multiply

```c
// x86: No direct intrinsic, need widening
__m128i q31_mul_sse(__m128i a, __m128i b) {
    __m128i lo = _mm_mul_epi32(a, b);        // Low 32 × low 32
    __m128i hi = _mm_mul_epi32(
        _mm_srli_epi64(a, 32),
        _mm_srli_epi64(b, 32)
    );
    // Extract bits 31-62 of 64-bit products
    lo = _mm_srli_epi64(lo, 31);
    hi = _mm_srli_epi64(hi, 31);
    return _mm_blend_epi32(lo, _mm_slli_epi64(hi, 32), 0xAA);
}

// NEON: vqdmulhq_s32 for Q31
int32x4_t q31_mul_neon(int32x4_t a, int32x4_t b) {
    return vqrdmulhq_s32(a, b);  // Round((a*b*2) >> 32)
}
```

---

## Platform Detection and Dispatch

### Compile-Time Detection
```c
#if defined(__AVX512BW__)
    #define SIMD_WIDTH 512
    #define Q15_MUL _mm512_mulhrs_epi16
#elif defined(__AVX2__)
    #define SIMD_WIDTH 256
    #define Q15_MUL _mm256_mulhrs_epi16
#elif defined(__SSSE3__)
    #define SIMD_WIDTH 128
    #define Q15_MUL _mm_mulhrs_epi16
#elif defined(__ARM_NEON)
    #define SIMD_WIDTH 128
    #define Q15_MUL(a, b) vqrdmulhq_s16(a, b)
#elif defined(__ARM_FEATURE_SVE2)
    #define SIMD_WIDTH -1  // VLA
    #define Q15_MUL svqrdmulh_s16
#elif defined(__riscv_v)
    #define SIMD_WIDTH -1  // Configurable
    // Use vmulh with appropriate rounding
#endif
```

### Runtime Detection (x86)
```c
#include <cpuid.h>

bool has_avx512bw(void) {
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 30)) != 0;  // AVX-512BW
}

bool has_avx2(void) {
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 5)) != 0;
}
```

---

## References

- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/)
- [ARM NEON Intrinsics Reference](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
- [ARM SVE Intrinsics](https://developer.arm.com/documentation/100891/latest)
- [RISC-V Vector Extension Spec](https://github.com/riscv/riscv-v-spec)
- [RISC-V Vector Intrinsics](https://github.com/riscv-non-isa/rvv-intrinsic-doc)
