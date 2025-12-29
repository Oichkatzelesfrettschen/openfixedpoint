# Formal Verification Tools for Fixed-Point Libraries

## Overview

Formal verification provides mathematical guarantees about program correctness. For fixed-point arithmetic, this means proving:
- Absence of overflow/underflow
- Bounds on numerical error
- Correctness of arithmetic operations
- Memory safety and absence of undefined behavior

---

## SMT Solvers

### Z3 (Microsoft Research)

**Installation (Ubuntu)**:
```bash
# Native Ubuntu package
sudo apt install z3 libz3-dev python3-z3

# Or via PyPI for latest version
pip install z3-solver
```

**Features**:
- Satisfiability Modulo Theories solver
- Supports bitvector theory (essential for fixed-point)
- Python, C, C++, and OCaml bindings
- MIT licensed (since 2015)

**Fixed-Point Application**:
```python
from z3 import *

# Verify Q15 multiplication doesn't overflow with saturation
a = BitVec('a', 16)
b = BitVec('b', 16)

# Q15 multiply: (a * b) >> 15
product = SignExt(16, a) * SignExt(16, b)
result = Extract(30, 15, product)

# Prove result fits in 16 bits (always true for Q15 * Q15)
solver = Solver()
solver.add(result > 32767)  # Try to find overflow
print(solver.check())  # Returns 'unsat' - no overflow possible
```

### CVC5

**Installation**:
```bash
# Build from source (not in Ubuntu repos)
git clone https://github.com/cvc5/cvc5
cd cvc5 && ./configure.sh && cd build && make
```

**Features**:
- Successor to CVC4
- Strong bitvector and integer support
- LFSC proof production
- BSD-3 licensed

### Alt-Ergo

**Installation**:
```bash
# Via OPAM (OCaml package manager)
opam install alt-ergo
```

**Features**:
- Used by Frama-C, SPARK, Why3
- Native support for fixed-point via bitvectors
- OCamlPro commercial support available

---

## Bounded Model Checkers

### CBMC (C Bounded Model Checker)

**Installation (Ubuntu)**:
```bash
# Official packages
sudo apt install cbmc

# Or from source for latest
git clone https://github.com/diffblue/cbmc
cd cbmc/src && make
```

**Features**:
- Supports C89, C99, C11, C17
- Bounded loops and recursion
- Memory safety checks
- Overflow/underflow detection
- Integration with Z3, CVC5, Boolector

**Fixed-Point Verification Example**:
```c
// verify_q16_add.c
#include <stdint.h>
#include <assert.h>

int32_t q16_16_add_sat(int32_t a, int32_t b) {
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > INT32_MAX) return INT32_MAX;
    if (sum < INT32_MIN) return INT32_MIN;
    return (int32_t)sum;
}

int main() {
    int32_t a, b;
    __CPROVER_assume(a >= INT32_MIN && a <= INT32_MAX);
    __CPROVER_assume(b >= INT32_MIN && b <= INT32_MAX);

    int32_t result = q16_16_add_sat(a, b);

    // Verify saturation works
    assert(result >= INT32_MIN);
    assert(result <= INT32_MAX);

    // Verify correctness when no overflow
    if ((int64_t)a + b <= INT32_MAX && (int64_t)a + b >= INT32_MIN) {
        assert(result == a + b);
    }

    return 0;
}
```

**Run**:
```bash
cbmc verify_q16_add.c --trace
```

### ESBMC (Efficient SMT-Based Model Checker)

**Installation (Ubuntu 24.04)**:
```bash
sudo apt install build-essential git cmake libz3-dev
git clone https://github.com/esbmc/esbmc
cd esbmc && mkdir build && cd build
cmake .. -DENABLE_Z3=1
make -j$(nproc)
```

**Features**:
- Fork of CBMC with enhancements
- Better floating-point support
- Multi-threaded program verification
- Active development

---

## Static Analyzers

### Frama-C

**Installation**:
```bash
# Via OPAM (recommended)
opam install frama-c

# Or Ubuntu PPA (may be outdated)
sudo add-apt-repository ppa:ocaml/opam
sudo apt update && sudo apt install frama-c
```

**Key Plugins**:

| Plugin | Purpose |
|--------|---------|
| Eva | Value analysis (abstract interpretation) |
| WP | Deductive verification (generates proof obligations) |
| Slicing | Code reduction for verification |
| RTE | Runtime error annotation generation |

**ACSL Annotations for Fixed-Point**:
```c
/*@ requires \valid(result);
    requires INT32_MIN <= a <= INT32_MAX;
    requires INT32_MIN <= b <= INT32_MAX;
    assigns *result;
    ensures *result == \max(INT32_MIN, \min(INT32_MAX, (integer)a + b));
*/
void q16_16_add_sat(int32_t a, int32_t b, int32_t *result);
```

### Fluctuat (Commercial)

**Features**:
- Abstract interpretation for numerical code
- Quantifies floating-point and fixed-point error
- Subdivision of ranges for tighter bounds
- Used in aerospace (Airbus, CNES)

### Astrée (Commercial, AbsInt)

**Features**:
- Proves absence of runtime errors
- Sound floating-point analysis
- Used in flight control software (Airbus A380)
- Handles fixed-point via integer analysis

### Polyspace (MathWorks, Commercial)

**Features**:
- MISRA compliance checking
- Proves absence of runtime errors
- Simulink integration
- Fixed-point toolbox support

---

## Proof Assistants

### Coq

**Installation**:
```bash
sudo apt install coq coqide
# Or via OPAM
opam install coq
```

**Use Cases**:
- CompCert (verified C compiler) uses Coq
- Prove correctness of algorithms
- Extract verified code to OCaml/Haskell

**Fixed-Point in Coq**:
```coq
(* Define Q16.16 as a record *)
Record Q16_16 := mkQ16 {
  value : Z;  (* Underlying integer *)
  valid : -2147483648 <= value <= 2147483647
}.

(* Prove addition is commutative *)
Theorem q16_add_comm : forall a b : Q16_16,
  q16_add a b = q16_add b a.
```

### Lean 4

**Installation**:
```bash
# Via elan (Lean version manager)
curl https://raw.githubusercontent.com/leanprover/elan/master/elan-init.sh -sSf | sh
```

**Features**:
- Modern dependently-typed language
- Good tactic support
- Active mathlib development
- Can generate executable code

### ACL2

**Features**:
- Used at AMD, Intel for hardware verification
- First-order logic
- Automated theorem proving
- Good for bit-level reasoning

---

## Testing Frameworks with Formal Flavors

### Catch2

**Installation**:
```bash
sudo apt install catch2
# Or header-only download
```

**Features**:
- BDD-style test cases
- Property-based testing via generators
- Micro-benchmarking

### GoogleTest + GoogleMock

**Installation**:
```bash
sudo apt install libgtest-dev libgmock-dev
```

### doctest

**Features**:
- Fastest compile times
- Header-only
- Compatible with Catch2 syntax

### RapidCheck (Property-Based Testing)

**Installation**:
```bash
git clone https://github.com/emil-e/rapidcheck
cd rapidcheck && mkdir build && cd build
cmake .. && make && sudo make install
```

**Fixed-Point Property Testing**:
```cpp
#include <rapidcheck.h>

rc::check("Q15 multiply is commutative", [](int16_t a, int16_t b) {
    RC_ASSERT(q15_mul(a, b) == q15_mul(b, a));
});

rc::check("Q15 multiply by 1 is identity (approx)", [](int16_t a) {
    int16_t one = 0x7FFF;  // ~1.0 in Q15
    int16_t result = q15_mul(a, one);
    RC_ASSERT(abs(result - a) <= 1);  // Allow 1 LSB error
});
```

---

## Gappa (Fixed-Point Specialist)

**Installation**:
```bash
# Build from source
git clone https://gitlab.inria.fr/gappa/gappa
cd gappa && ./autogen.sh && ./configure && make
```

**Features**:
- Specifically designed for fixed-point/floating-point bounds
- Proves error bounds on numerical computations
- Used to verify CRlibm elementary functions
- Generates Coq proofs

**Example (Q16.16 Error Bound)**:
```gappa
# Define the Q16.16 format
@fix<-16,ne> = fixed<-16, ne>;

# Variables
x = fix<-16,ne>(X);
y = fix<-16,ne>(Y);

# Compute multiplication
z = x * y;
z_rounded fix<-16,ne>= z;

# Prove error bound
{ X in [-1, 1] /\ Y in [-1, 1] ->
  |z_rounded - X*Y| <= 0x1p-15 }
```

---

## Symbolic Execution

### KLEE

**Installation**:
```bash
sudo apt install klee llvm clang
```

**Features**:
- LLVM-based symbolic execution
- Generates test cases for high coverage
- Finds assertion violations

**Usage**:
```bash
clang -emit-llvm -c -g q16_mul.c -o q16_mul.bc
klee --emit-all-errors q16_mul.bc
```

### SAGE (Microsoft, Internal)

- Whitebox fuzzing with symbolic execution
- Found ~1/3 of Windows 7 security bugs

---

## CI Integration Strategy

### GitHub Actions Workflow

```yaml
name: Formal Verification

on: [push, pull_request]

jobs:
  cbmc:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install CBMC
        run: sudo apt-get install -y cbmc
      - name: Verify Q16.16 operations
        run: |
          cbmc src/q16_16.c --function q16_16_add_sat --unwind 10
          cbmc src/q16_16.c --function q16_16_mul --unwind 10

  z3-proofs:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Z3
        run: pip install z3-solver
      - name: Run Z3 proofs
        run: python tests/formal/z3_proofs.py

  property-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build with RapidCheck
        run: |
          cmake -B build -DENABLE_PROPERTY_TESTS=ON
          cmake --build build
      - name: Run property tests
        run: ./build/tests/property_tests --iterations 100000
```

---

## Recommended Verification Strategy for libfixp

### Tier 1: Always (CI)
- **CBMC**: Bounded model checking for core operations
- **RapidCheck**: Property-based testing
- **AddressSanitizer/UBSan**: Runtime checks

### Tier 2: Release Verification
- **Gappa**: Prove error bounds
- **Frama-C/Eva**: Value analysis
- **Z3 scripts**: Custom proofs for tricky cases

### Tier 3: Deep Verification (Optional)
- **Coq/Lean**: Full formal proofs
- **CompCert**: Compile with verified compiler

---

## References

- [CBMC User Manual](http://www.cprover.org/cbmc/doc/cbmc-manual.html)
- [Frama-C Documentation](https://frama-c.com/html/documentation.html)
- [Z3 Tutorial](https://microsoft.github.io/z3guide/)
- [Gappa Documentation](https://gappa.gitlabpages.inria.fr/)
- [RapidCheck Docs](https://github.com/emil-e/rapidcheck/blob/master/doc/user_guide.md)
- [Agner Fog Optimization Resources](https://www.agner.org/optimize/)
