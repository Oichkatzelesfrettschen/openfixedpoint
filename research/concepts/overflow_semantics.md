# Overflow and Saturation Semantics

## Overview

When fixed-point operations produce results outside representable range, the system must respond. Four primary policies exist, each with distinct tradeoffs.

## Policy Types

### 1. Wrapping (Modular Arithmetic)
- Behavior: Result wraps around (e.g., INT_MAX + 1 = INT_MIN)
- Pros: Fast, deterministic, matches C integer behavior
- Cons: Silent corruption, wrong results
- Use case: Crypto, hash functions, low-level bit manipulation

### 2. Saturating
- Behavior: Clamps to min/max representable value
- Pros: Graceful degradation, audio/video friendly
- Cons: Still loses information, slight overhead
- Use case: DSP, audio processing, control systems
- Hardware: ARM QADD/QSUB, x86 PADDS/PADDUS

### 3. Trapping
- Behavior: Raises exception/signal on overflow
- Pros: Immediate error detection
- Cons: Performance cost, requires handling
- Use case: Financial, safety-critical systems

### 4. Undefined Behavior (UB)
- Behavior: Compiler assumes overflow never happens
- Pros: Maximum optimization opportunity
- Cons: Dangerous, unpredictable in overflow case
- Use case: Hot loops where overflow is provably impossible

## Hardware Support

### ARM (ACLE)
- Q flag (sticky saturation indicator)
- `__qadd()`, `__qsub()` - saturating add/subtract
- `__ssat()`, `__usat()` - width-specified saturation
- `__saturation_occurred()` - check Q flag
- Deprecated on A-profile, supported M/R-profile

### x86/x64 (SSE/AVX)
- `_mm_adds_epi8/16` - signed saturating add
- `_mm_adds_epu8/16` - unsigned saturating add
- `_mm_subs_epi8/16` - signed saturating subtract
- No native trap support

### AVX-512BW
- `_mm512_adds_epi8/16`, `_mm512_subs_epi8/16`
- Mask registers for conditional operations

## Novel Design: Runtime Policy Switching

### Requirements
1. Build-time configuration (default policy)
2. Runtime switching without restart
3. Per-operation policy override
4. Zero overhead when not switching

### Implementation Strategies

#### Strategy A: Policy as Template Parameter
```cpp
template<OverflowPolicy P = Saturate>
struct fixp { ... };

// Fast, but requires different types for different policies
```

#### Strategy B: Policy as Runtime Parameter
```cpp
fixp<16,15> a, b;
auto c = add(a, b, Policy::Wrap);
```

#### Strategy C: Thread-Local Default
```cpp
thread_local OverflowPolicy g_policy = Saturate;
set_overflow_policy(Policy::Trap);
// All subsequent operations use new policy
```

#### Strategy D: Hybrid (Recommended for libfixp)
- Compile-time default via template
- Thread-local override for runtime switching
- Per-operation override for fine control
- Inline helpers that check policy at zero cost when static

### Zero-Overhead Switching Design

```cpp
enum class OverflowPolicy { Wrap, Saturate, Trap, UB };

// Compile-time known policy - fully inlined
template<OverflowPolicy P>
constexpr auto add_impl(auto a, auto b) {
    if constexpr (P == Wrap) { return a + b; }
    else if constexpr (P == Saturate) { return sat_add(a, b); }
    // ...
}

// Runtime policy - branch prediction helps
inline auto add_runtime(auto a, auto b, OverflowPolicy p) {
    switch(p) { /* dispatch */ }
}
```

## Saturation Detection

### Post-hoc Detection
- Compare result sign with expected (based on operand signs)
- Check if result < min operand (for addition of same-sign values)

### Inline Saturation (ARM style)
```cpp
int32_t sat_add(int32_t a, int32_t b) {
    int64_t result = (int64_t)a + b;
    if (result > INT32_MAX) return INT32_MAX;
    if (result < INT32_MIN) return INT32_MIN;
    return (int32_t)result;
}
```

### Branchless Saturation (Advanced)
Using arithmetic tricks to avoid branches in hot paths.

## Design Decisions for libfixp

1. **Default policy**: Saturating (DSP-friendly)
2. **Template parameter**: Optional policy override
3. **Runtime API**: `set_overflow_policy()` thread-local
4. **Per-op override**: `add<Policy::Wrap>(a, b)`
5. **Detection API**: `last_overflow()` sticky flag
