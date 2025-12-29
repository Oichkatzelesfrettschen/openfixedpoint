# Real-World Fixed-Point Case Studies

## Game Engines

### id Software: Doom (1993)

**Format**: Q16.16 (32-bit total)

**Why Fixed-Point**:
- Target hardware: 386/486 without FPU (or slow 387 coprocessor)
- Integer instructions were 10-50x faster than FP
- Deterministic gameplay essential for demo recording/playback

**Implementation Details**:
```c
// From Doom source (simplified)
typedef int32_t fixed_t;

#define FRACBITS        16
#define FRACUNIT        (1 << FRACBITS)  // 65536

// Multiply: full 64-bit intermediate
fixed_t FixedMul(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * b) >> FRACBITS);
}

// Divide: shift up first to preserve precision
fixed_t FixedDiv(fixed_t a, fixed_t b) {
    if ((abs(a) >> 14) >= abs(b)) {
        return (a ^ b) < 0 ? INT32_MIN : INT32_MAX;  // Overflow
    }
    return (fixed_t)(((int64_t)a << FRACBITS) / b);
}
```

**Range Used**:
- Map coordinates: typically -32768 to +32767 units
- Fractional precision: 1/65536 unit (sub-pixel)
- View angles: 0 to 2^32 (full rotation)

**Lessons**:
- Q16.16 sufficient for indoor game environments
- Careful overflow checking in division
- Still used in modern source ports (GZDoom, etc.)

### id Software: Quake (1996)

**Transition to Floating-Point**

**Why the Change**:
- Pentium FPU was finally fast (pipelined)
- 3D rendering with perspective division
- Larger outdoor environments needed more range

**Hybrid Approach**:
- Integer for BSP traversal
- Float for 3D transforms
- Clever FPU pipeline scheduling

**The Carmack Trick**:
```c
// Use FPU division asynchronously
// Start divide, do integer work, collect result later
float reciprocal = 1.0f / z;  // Start FPU divide
// ... 8 pixels of integer texture mapping ...
// Now reciprocal is ready
```

### Sony PlayStation 1 (1994)

**Format**: Q12.4 and Q1.19.12 (GTE coprocessor)

**Why These Formats**:
- GTE (Geometry Transform Engine) used fixed-point
- Q12.4 for screen coordinates (4096x4096 max)
- Q1.19.12 for 3D transforms

**Challenges**:
- "Wobbling" textures due to lack of sub-pixel precision
- Z-fighting from limited depth precision
- Developers had to carefully manage coordinate ranges

### Nintendo 64 / GameCube

**Format**: Q16.16 and custom per-game

**Examples**:
- Super Mario 64: Q16.16 for positions
- Zelda OoT: Mixed fixed/float (N64 had slow FPU)
- GameCube: Full float (hardware FPU)

---

## Digital Signal Processing

### Audio Codecs

#### MP3 (MPEG Audio Layer III)

**Fixed-Point Implementations**:
- libmad: Q28 fixed-point for 32-bit ARM
- Helix: Q15/Q31 for low-memory devices

**Why Fixed-Point**:
- Many target devices had no FPU (early MP3 players)
- Deterministic decoding for DRM
- Power efficiency on mobile

**Precision Requirements**:
```
DCT coefficients: Q31 (high precision needed)
Filter banks: Q15 (acceptable for audio)
Scale factors: Integer (exponents)
```

#### AAC/Opus

**Modern Approach**: Float reference, fixed-point for embedded

**Opus Codec**:
```c
// From Opus source: Q15 multiply
static inline int16_t SIG_SAT16(int32_t x) {
    return x > 32767 ? 32767 : (x < -32768 ? -32768 : x);
}

// SILK encoder uses Q16 for LPC coefficients
```

### Digital Filters (IIR/FIR)

**Typical Format**: Q15 for audio, Q31 for high-precision

**CMSIS-DSP Implementation**:
```c
// Q15 biquad filter (ARM optimized)
void arm_biquad_cascade_df1_q15(
    const arm_biquad_casd_df1_inst_q15 * S,
    q15_t * pSrc,
    q15_t * pDst,
    uint32_t blockSize);

// Uses saturation and extended accumulator
// 64-bit accumulator for 32 Q15 coefficients
```

**Coefficient Scaling**:
- Scale coefficients to prevent overflow
- Sum of |coefficients| <= 1.0 (safe)
- Or use block floating-point per filter section

### Software-Defined Radio (SDR)

**Format**: Q15 for samples, Q31 for accumulators

**Examples**:
- GNU Radio: Float internally, Q15 for hardware I/O
- RTL-SDR: 8-bit I/Q samples from hardware
- LimeSDR: 12-bit samples

**Mixing (NCO)**:
```c
// Numerically Controlled Oscillator in Q15
int16_t nco_phase;  // Q16 angle (full rotation = 65536)
int16_t nco_freq;   // Phase increment per sample

int16_t nco_next(void) {
    nco_phase += nco_freq;
    return sin_table_q15[nco_phase >> 8];  // 256-entry table
}
```

---

## Embedded Systems

### Automotive (AUTOSAR, ISO 26262)

**Requirements**:
- Deterministic execution (WCET analysis)
- Bit-exact reproducibility
- No dynamic memory allocation

**Typical Formats**:
```
Sensor inputs: Q12.4 or Q8.8
Control loops: Q16.16 or Q1.31
Actuator outputs: Q12.4
```

**Engine Control Unit (ECU)**:
```c
// Fuel injection timing calculation
// Q16.16 for timing, Q8.8 for temperature compensation
q16_16 injection_time = base_time;
injection_time = q16_16_mul(injection_time, temp_correction);
injection_time = q16_16_add(injection_time, altitude_offset);
```

### Industrial Control (PLC)

**IEC 61131-3 Data Types**:
```
SINT:  8-bit signed
INT:  16-bit signed
DINT: 32-bit signed
LINT: 64-bit signed
```

**Fixed-Point in PLCs**:
- Often implicit scaling (e.g., divide by 1000)
- Q16.16 for PID controllers
- Integer-only for safety certification

### Aerospace (DO-178C)

**Requirements**:
- Level A: Catastrophic failure prevention
- Determinism mandatory
- Proven numerical bounds

**ARINC 429 Data**:
- 32-bit word format
- Fixed-point with defined scaling per label
- BCD for some parameters

**Example (Flight Control)**:
```c
// Pitch rate in Q16.16 (degrees/second)
// Range: -180 to +180 deg/s
// Resolution: 0.0000153 deg/s

q16_16 pitch_rate = sensor_read_q16_16(PITCH_GYRO);
q16_16 command = q16_16_mul(pitch_rate, gain);
// Saturate to actuator limits
command = q16_16_clamp(command, MIN_ELEVATOR, MAX_ELEVATOR);
```

### Medical Devices (IEC 62304)

**Requirements**:
- Predictable behavior
- Verified arithmetic
- No undefined behavior

**ECG Processing**:
```c
// 12-bit ADC input, Q12 format
// Filter to Q15 for processing
// Display in Q8.8 for GUI

q15_t ecg_filter(q12_t sample) {
    static q31_t acc = 0;  // Extended precision accumulator
    acc = acc - (acc >> 4) + ((q31_t)sample << 3);
    return (q15_t)(acc >> 4);  // IIR lowpass
}
```

---

## Financial Systems

### Currency Representation

**Why NOT Fixed-Point (Usually)**:
- Prefer decimal fixed-point or integer cents
- Binary fixed-point causes rounding surprises
- Regulatory requirements for decimal rounding

**Exception: High-Frequency Trading**:
```c
// Prices in "ticks" (integer)
// Quantities as integer shares
// No fractional shares or sub-tick prices

typedef int64_t price_t;   // Ticks
typedef int64_t qty_t;     // Shares

// Extended precision for notional value
typedef __int128 value_t;  // ticks × shares

value_t notional(price_t p, qty_t q) {
    return (value_t)p * q;
}
```

### Risk Calculations

**Where Fixed-Point Helps**:
- Reproducibility across systems
- Audit trail (bit-exact)
- Parallel computation without race conditions

---

## Machine Learning Inference

### Quantization

**INT8 Inference**:
```
Original: float32 weights and activations
Quantized: int8 weights, int8 or int16 activations
Scale factors: per-tensor or per-channel
```

**TensorFlow Lite Quantization**:
```
real_value = (int8_value - zero_point) * scale

Q-format interpretation:
- int8_value: Q7 with zero_point offset
- scale: float32 (or also quantized)
```

### MX Format (Microscaling)

**Block Floating-Point for AI**:
```
Block: 32 elements
Storage: 32 × 8-bit mantissa + 1 × 8-bit shared exponent
Bits/element: 8.25 average
```

**Hardware Support**:
- AMD MI300X
- Intel Gaudi3
- NVIDIA Blackwell (planned)

---

## Key Lessons from Case Studies

### 1. Format Selection Matters

| Domain | Typical Format | Why |
|--------|---------------|-----|
| Retro games | Q16.16 | 32-bit integers, no FPU |
| Audio DSP | Q15/Q31 | Match ADC/DAC precision |
| Embedded control | Q12.4 to Q16.16 | Determinism, WCET |
| ML inference | INT8/INT16 | Throughput, power |

### 2. Overflow Handling Varies

| Domain | Strategy |
|--------|----------|
| Games | Saturation (visual artifacts > crash) |
| Audio | Saturation (clipping > silence) |
| Control | Saturation + alarm flag |
| Safety-critical | Detect and halt |

### 3. Precision Accumulation

**Pattern**: Use wider accumulator, narrow at end
```c
// Q15 FIR filter with Q31 accumulator
q31_t acc = 0;
for (int i = 0; i < taps; i++) {
    acc += (q31_t)samples[i] * coeffs[i];  // Q30 product
}
return (q15_t)(acc >> 15);  // Round to Q15
```

### 4. Conversion Boundaries

**Best Practice**: Convert at I/O boundaries only
- Input: ADC/sensor → fixed-point (native)
- Processing: Stay in fixed-point
- Output: Fixed-point → DAC/actuator (native)

**Anti-Pattern**: Repeated float↔fixed conversion
- Each conversion loses precision
- Adds latency
- Wastes power

---

## References

- [Doom Source Code](https://github.com/id-Software/DOOM)
- [Quake Source Code](https://github.com/id-Software/Quake)
- [libmad MP3 Decoder](https://www.underbit.com/products/mad/)
- [Opus Codec](https://opus-codec.org/)
- [CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP)
- [TensorFlow Lite Quantization](https://www.tensorflow.org/lite/performance/quantization_spec)
- [DoomWiki: Fixed Point](https://doomwiki.org/wiki/Fixed_point)
