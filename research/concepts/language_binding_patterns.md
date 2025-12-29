# Language Binding Patterns for Fixed-Point Libraries

## C++ Template Metaprogramming

### Basic Type System with Templates

```cpp
// Fixed-point type parameterized by total bits and fractional bits
template<int TotalBits, int FracBits>
struct Fixed {
    static_assert(TotalBits == 8 || TotalBits == 16 ||
                  TotalBits == 32 || TotalBits == 64,
                  "TotalBits must be 8, 16, 32, or 64");
    static_assert(FracBits >= 0 && FracBits < TotalBits,
                  "FracBits must be in [0, TotalBits)");

    // Select underlying type based on TotalBits
    using StorageType = std::conditional_t<TotalBits == 8, int8_t,
                        std::conditional_t<TotalBits == 16, int16_t,
                        std::conditional_t<TotalBits == 32, int32_t,
                        int64_t>>>;

    // Double-width type for intermediate calculations
    using WideType = std::conditional_t<TotalBits == 8, int16_t,
                     std::conditional_t<TotalBits == 16, int32_t,
                     std::conditional_t<TotalBits == 32, int64_t,
                     __int128>>>;

    StorageType value;

    // Constants
    static constexpr int total_bits = TotalBits;
    static constexpr int frac_bits = FracBits;
    static constexpr int int_bits = TotalBits - FracBits - 1;  // Minus sign bit
    static constexpr StorageType one = StorageType(1) << FracBits;

    // Constructors
    constexpr Fixed() : value(0) {}
    constexpr explicit Fixed(StorageType raw) : value(raw) {}

    // From floating point (constexpr in C++20)
    constexpr explicit Fixed(double d)
        : value(static_cast<StorageType>(d * one + (d >= 0 ? 0.5 : -0.5))) {}

    // To floating point
    constexpr double to_double() const {
        return static_cast<double>(value) / one;
    }
};

// Type aliases
using Q7 = Fixed<8, 7>;
using Q15 = Fixed<16, 15>;
using Q16_16 = Fixed<32, 16>;
using Q1_31 = Fixed<32, 31>;
using Q32_32 = Fixed<64, 32>;
```

### Operator Overloading

```cpp
template<int T, int F>
constexpr Fixed<T, F> operator+(Fixed<T, F> a, Fixed<T, F> b) {
    return Fixed<T, F>(a.value + b.value);
}

template<int T, int F>
constexpr Fixed<T, F> operator-(Fixed<T, F> a, Fixed<T, F> b) {
    return Fixed<T, F>(a.value - b.value);
}

template<int T, int F>
constexpr Fixed<T, F> operator*(Fixed<T, F> a, Fixed<T, F> b) {
    using Wide = typename Fixed<T, F>::WideType;
    Wide product = static_cast<Wide>(a.value) * b.value;
    // Round to nearest
    product += Wide(1) << (F - 1);
    return Fixed<T, F>(static_cast<typename Fixed<T, F>::StorageType>(product >> F));
}

template<int T, int F>
constexpr Fixed<T, F> operator/(Fixed<T, F> a, Fixed<T, F> b) {
    using Wide = typename Fixed<T, F>::WideType;
    Wide dividend = static_cast<Wide>(a.value) << F;
    return Fixed<T, F>(static_cast<typename Fixed<T, F>::StorageType>(dividend / b.value));
}
```

### Saturating Arithmetic with Policy

```cpp
// Overflow policies
struct Wrap {};
struct Saturate {};
struct Trap {};

template<int T, int F, typename Policy = Wrap>
struct FixedSafe {
    using Base = Fixed<T, F>;
    typename Base::StorageType value;

    // Saturating add
    constexpr FixedSafe add(FixedSafe other) const {
        if constexpr (std::is_same_v<Policy, Saturate>) {
            using Wide = typename Base::WideType;
            Wide sum = static_cast<Wide>(value) + other.value;
            if (sum > std::numeric_limits<typename Base::StorageType>::max())
                return FixedSafe{std::numeric_limits<typename Base::StorageType>::max()};
            if (sum < std::numeric_limits<typename Base::StorageType>::min())
                return FixedSafe{std::numeric_limits<typename Base::StorageType>::min()};
            return FixedSafe{static_cast<typename Base::StorageType>(sum)};
        } else if constexpr (std::is_same_v<Policy, Trap>) {
            typename Base::StorageType result;
            if (__builtin_add_overflow(value, other.value, &result))
                std::abort();
            return FixedSafe{result};
        } else {
            return FixedSafe{static_cast<typename Base::StorageType>(value + other.value)};
        }
    }
};
```

### C++20 Concepts for Generic Programming

```cpp
template<typename T>
concept FixedPointType = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    { a - b } -> std::same_as<T>;
    { a * b } -> std::same_as<T>;
    { a.value } -> std::integral;
    { T::frac_bits } -> std::convertible_to<int>;
};

template<FixedPointType T>
T sin_approx(T angle) {
    // Generic sin approximation for any fixed-point type
    // Implementation uses T::frac_bits for scaling
}
```

### Constexpr Everything (C++17/20)

```cpp
// Compile-time lookup table generation
template<int Size>
constexpr std::array<Q15, Size> generate_sin_table() {
    std::array<Q15, Size> table{};
    for (int i = 0; i < Size; i++) {
        double angle = 2.0 * 3.14159265358979 * i / Size;
        table[i] = Q15(std::sin(angle));
    }
    return table;
}

// Instantiated at compile time
inline constexpr auto sin_table_256 = generate_sin_table<256>();
```

---

## Rust Traits and Generic Arithmetic

### Core Trait Definitions

```rust
use std::ops::{Add, Sub, Mul, Div, Neg};

pub trait FixedPoint: Copy + Sized {
    type Storage: Copy;
    type Wide: Copy;

    const FRAC_BITS: u32;
    const TOTAL_BITS: u32;
    const ONE: Self::Storage;

    fn from_raw(raw: Self::Storage) -> Self;
    fn raw(self) -> Self::Storage;

    fn from_f64(f: f64) -> Self;
    fn to_f64(self) -> f64;
}
```

### Implementing for Q16.16

```rust
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
#[repr(transparent)]
pub struct Q16_16(i32);

impl FixedPoint for Q16_16 {
    type Storage = i32;
    type Wide = i64;

    const FRAC_BITS: u32 = 16;
    const TOTAL_BITS: u32 = 32;
    const ONE: i32 = 1 << 16;

    #[inline]
    fn from_raw(raw: i32) -> Self {
        Q16_16(raw)
    }

    #[inline]
    fn raw(self) -> i32 {
        self.0
    }

    fn from_f64(f: f64) -> Self {
        Q16_16((f * Self::ONE as f64).round() as i32)
    }

    fn to_f64(self) -> f64 {
        self.0 as f64 / Self::ONE as f64
    }
}

impl Add for Q16_16 {
    type Output = Self;
    #[inline]
    fn add(self, rhs: Self) -> Self {
        Q16_16(self.0.wrapping_add(rhs.0))
    }
}

impl Mul for Q16_16 {
    type Output = Self;
    #[inline]
    fn mul(self, rhs: Self) -> Self {
        let product = (self.0 as i64) * (rhs.0 as i64);
        Q16_16(((product + (1 << 15)) >> 16) as i32)
    }
}
```

### Saturating Operations

```rust
impl Q16_16 {
    #[inline]
    pub fn saturating_add(self, rhs: Self) -> Self {
        Q16_16(self.0.saturating_add(rhs.0))
    }

    #[inline]
    pub fn saturating_sub(self, rhs: Self) -> Self {
        Q16_16(self.0.saturating_sub(rhs.0))
    }

    #[inline]
    pub fn saturating_mul(self, rhs: Self) -> Self {
        let product = (self.0 as i64) * (rhs.0 as i64);
        let shifted = (product + (1 << 15)) >> 16;
        Q16_16(shifted.clamp(i32::MIN as i64, i32::MAX as i64) as i32)
    }
}
```

### The `fixed` Crate Pattern

From [docs.rs/fixed](https://docs.rs/fixed):

```rust
use fixed::types::I16F16;  // Same as Q16.16

let a = I16F16::from_num(2.5);
let b = I16F16::from_num(1.5);
let c = a * b;  // 3.75
let d = a.saturating_mul(b);  // With saturation

// Generic over fixed-point types
use fixed::traits::Fixed;

fn lerp<F: Fixed>(a: F, b: F, t: F) -> F {
    a + (b - a) * t
}
```

### Const Generics (Rust 1.51+)

```rust
use std::marker::PhantomData;

pub struct Fixed<const FRAC: u32, S> {
    value: S,
    _phantom: PhantomData<()>,
}

// Q8.8
pub type Q8_8 = Fixed<8, i16>;
// Q16.16
pub type Q16_16_Alt = Fixed<16, i32>;
// Q32.32
pub type Q32_32 = Fixed<32, i64>;

impl<const F: u32, S: Copy> Fixed<F, S>
where
    S: From<i32> + Into<i64>,
{
    pub fn from_raw(raw: S) -> Self {
        Fixed { value: raw, _phantom: PhantomData }
    }
}
```

---

## Python Bindings

### ctypes (Standard Library)

```python
import ctypes

# Load shared library
libfixp = ctypes.CDLL("./libfixp.so")

# Define Q16.16 type
class Q16_16(ctypes.Structure):
    _fields_ = [("value", ctypes.c_int32)]

    def __init__(self, val=0):
        if isinstance(val, float):
            self.value = int(val * 65536)
        else:
            self.value = val

    def to_float(self):
        return self.value / 65536.0

    def __repr__(self):
        return f"Q16_16({self.to_float():.6f})"

# Define function signatures
libfixp.q16_16_add.argtypes = [Q16_16, Q16_16]
libfixp.q16_16_add.restype = Q16_16

libfixp.q16_16_mul.argtypes = [Q16_16, Q16_16]
libfixp.q16_16_mul.restype = Q16_16

# Usage
a = Q16_16(2.5)
b = Q16_16(1.5)
c = libfixp.q16_16_mul(a, b)
print(c)  # Q16_16(3.750000)
```

### cffi (Better for Complex Types)

```python
from cffi import FFI

ffi = FFI()

# Define C declarations
ffi.cdef("""
    typedef int32_t q16_16;

    q16_16 q16_16_add(q16_16 a, q16_16 b);
    q16_16 q16_16_sub(q16_16 a, q16_16 b);
    q16_16 q16_16_mul(q16_16 a, q16_16 b);
    q16_16 q16_16_div(q16_16 a, q16_16 b);

    q16_16 sin_q16_16(q16_16 angle);
    q16_16 cos_q16_16(q16_16 angle);
""")

# Load library
lib = ffi.dlopen("./libfixp.so")

# Python wrapper class
class FixedPoint:
    SCALE = 65536

    def __init__(self, value=0):
        if isinstance(value, float):
            self._raw = int(value * self.SCALE)
        else:
            self._raw = value

    @classmethod
    def from_raw(cls, raw):
        fp = cls.__new__(cls)
        fp._raw = raw
        return fp

    def __add__(self, other):
        return FixedPoint.from_raw(lib.q16_16_add(self._raw, other._raw))

    def __mul__(self, other):
        return FixedPoint.from_raw(lib.q16_16_mul(self._raw, other._raw))

    def __float__(self):
        return self._raw / self.SCALE
```

### pybind11 (Modern C++/Python Bridge)

```cpp
// bindings.cpp
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "fixed.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pyfixp, m) {
    m.doc() = "Fixed-point arithmetic library";

    py::class_<Fixed<32, 16>>(m, "Q16_16")
        .def(py::init<>())
        .def(py::init<double>())
        .def(py::init<int32_t>())
        .def("to_float", &Fixed<32, 16>::to_double)
        .def_property_readonly("raw", [](const Fixed<32, 16>& f) {
            return f.value;
        })
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(-py::self)
        .def("__repr__", [](const Fixed<32, 16>& f) {
            return "Q16_16(" + std::to_string(f.to_double()) + ")";
        });

    // Math functions
    m.def("sin", &sin_q16_16, "Sine of fixed-point angle");
    m.def("cos", &cos_q16_16, "Cosine of fixed-point angle");
    m.def("sqrt", &sqrt_q16_16, "Square root");
}
```

### NumPy Integration

```python
import numpy as np

# Custom dtype for Q16.16
q16_16_dtype = np.dtype([('value', np.int32)])

def to_q16_16(arr):
    """Convert float array to Q16.16."""
    return (arr * 65536).astype(np.int32)

def from_q16_16(arr):
    """Convert Q16.16 array to float."""
    return arr.astype(np.float64) / 65536

# Vectorized operations using NumPy
def q16_16_mul_vec(a, b):
    """Q16.16 multiply for arrays."""
    product = a.astype(np.int64) * b.astype(np.int64)
    return ((product + 0x8000) >> 16).astype(np.int32)

# Example
a = to_q16_16(np.array([1.0, 2.0, 3.0]))
b = to_q16_16(np.array([0.5, 0.5, 0.5]))
c = q16_16_mul_vec(a, b)
print(from_q16_16(c))  # [0.5, 1.0, 1.5]
```

---

## C Interoperability Layer

### Header Design for Multi-Language Support

```c
// libfixp.h - C API header

#ifndef LIBFIXP_H
#define LIBFIXP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Type definitions
typedef int8_t  q7_t;
typedef int16_t q15_t;
typedef int32_t q16_16_t;
typedef int32_t q1_31_t;
typedef int64_t q32_32_t;

// Q16.16 operations
q16_16_t q16_16_add(q16_16_t a, q16_16_t b);
q16_16_t q16_16_sub(q16_16_t a, q16_16_t b);
q16_16_t q16_16_mul(q16_16_t a, q16_16_t b);
q16_16_t q16_16_div(q16_16_t a, q16_16_t b);

// Saturating variants
q16_16_t q16_16_add_sat(q16_16_t a, q16_16_t b);
q16_16_t q16_16_sub_sat(q16_16_t a, q16_16_t b);
q16_16_t q16_16_mul_sat(q16_16_t a, q16_16_t b);

// Math functions
q16_16_t sin_q16_16(q16_16_t angle);
q16_16_t cos_q16_16(q16_16_t angle);
q16_16_t sqrt_q16_16(q16_16_t x);
q16_16_t atan2_q16_16(q16_16_t y, q16_16_t x);

// Conversion
q16_16_t double_to_q16_16(double d);
double q16_16_to_double(q16_16_t q);

#ifdef __cplusplus
}
#endif

// C++ wrapper when compiled as C++
#ifdef __cplusplus
#include <type_traits>

namespace fixp {
    // Modern C++ wrapper using the C API
    class Q16_16 {
        q16_16_t value_;
    public:
        constexpr Q16_16() : value_(0) {}
        explicit Q16_16(double d) : value_(double_to_q16_16(d)) {}
        explicit Q16_16(q16_16_t raw) : value_(raw) {}

        q16_16_t raw() const { return value_; }
        double to_double() const { return q16_16_to_double(value_); }

        Q16_16 operator+(Q16_16 other) const {
            return Q16_16(q16_16_add(value_, other.value_));
        }
        // ... etc
    };
}
#endif

#endif // LIBFIXP_H
```

---

## WebAssembly Bindings

### Emscripten

```cpp
// bindings_wasm.cpp
#include <emscripten/bind.h>
#include "fixed.hpp"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(fixp) {
    class_<Fixed<32, 16>>("Q16_16")
        .constructor<>()
        .constructor<double>()
        .function("toFloat", &Fixed<32, 16>::to_double)
        .property("raw", &Fixed<32, 16>::value);

    function("q16_add", +[](Fixed<32, 16> a, Fixed<32, 16> b) {
        return a + b;
    });
    function("q16_mul", +[](Fixed<32, 16> a, Fixed<32, 16> b) {
        return a * b;
    });
    function("sin_q16", &sin_q16_16);
}
```

### AssemblyScript

```typescript
// fixp.ts (AssemblyScript)
@unmanaged
export class Q16_16 {
    value: i32;

    constructor(v: f64 = 0) {
        this.value = <i32>(v * 65536.0 + 0.5);
    }

    static fromRaw(raw: i32): Q16_16 {
        let q = new Q16_16(0);
        q.value = raw;
        return q;
    }

    toFloat(): f64 {
        return <f64>this.value / 65536.0;
    }

    add(other: Q16_16): Q16_16 {
        return Q16_16.fromRaw(this.value + other.value);
    }

    mul(other: Q16_16): Q16_16 {
        let product = <i64>this.value * <i64>other.value;
        return Q16_16.fromRaw(<i32>((product + 0x8000) >> 16));
    }
}
```

---

## Summary: Binding Strategy for libfixp

| Language | Approach | Complexity | Performance |
|----------|----------|------------|-------------|
| C | Native header | Low | Best |
| C++ | Header + templates | Medium | Best |
| Rust | FFI or pure reimpl | Medium | Near-native |
| Python | pybind11 or cffi | Medium | Good |
| JavaScript | WASM + Emscripten | Medium | Good |
| Go | cgo | Low | Good |
| Zig | @cImport | Low | Near-native |

**Recommended Strategy**:
1. Core in C (header-only where possible)
2. C++ template wrapper in same header
3. pybind11 for Python bindings
4. `fixed` crate compatible API for Rust
5. WASM for web via Emscripten

---

## References

- [fixed crate (Rust)](https://docs.rs/fixed)
- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [cffi Documentation](https://cffi.readthedocs.io/)
- [Emscripten Embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html)
- [C++ Core Guidelines: Templates](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#t-templates-and-generic-programming)
