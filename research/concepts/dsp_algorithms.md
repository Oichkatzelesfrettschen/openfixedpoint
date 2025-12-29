# DSP Algorithms in Fixed-Point

## FIR Filters

### Structure
```
y[n] = Σ(h[k] × x[n-k]) for k = 0 to N-1
```

### Fixed-Point Implementation
```c
int16_t fir_q15(int16_t* x, int16_t* h, int N) {
    int32_t acc = 0;  // Q31 accumulator
    for (int k = 0; k < N; k++) {
        acc += (int32_t)x[k] * h[k];  // Q15 × Q15 = Q30
    }
    // Round and saturate to Q15
    acc = (acc + 0x4000) >> 15;  // Round
    if (acc > 32767) return 32767;
    if (acc < -32768) return -32768;
    return (int16_t)acc;
}
```

### Coefficient Scaling
Coefficients must sum to ≤ 1.0 to prevent overflow:
```
Σ|h[k]| ≤ 1.0 (safe)
Σ|h[k]| > 1.0 (needs headroom)
```

### Block Processing
For long filters, use overlap-add/overlap-save with FFT.

---

## IIR Filters

### Direct Form II Transposed
```
w[n] = x[n] + a1×w[n-1] + a2×w[n-2]
y[n] = b0×w[n] + b1×w[n-1] + b2×w[n-2]
```

### Fixed-Point Challenges
1. **Pole sensitivity**: Small coefficient errors shift poles
2. **State variable precision**: Need more bits than I/O
3. **Limit cycles**: Quantization causes unwanted oscillations

### Biquad Implementation (Second Order Section)
```c
typedef struct {
    int32_t w1, w2;  // State (extra precision)
    int16_t b0, b1, b2, a1, a2;  // Coefficients (Q14)
} biquad_t;

int16_t biquad_process(biquad_t* f, int16_t x) {
    // DF2T structure
    int32_t w0 = ((int32_t)x << 14)
               - ((int32_t)f->a1 * f->w1 >> 14)
               - ((int32_t)f->a2 * f->w2 >> 14);

    int32_t y = ((int32_t)f->b0 * w0 >> 14)
              + ((int32_t)f->b1 * f->w1 >> 14)
              + ((int32_t)f->b2 * f->w2 >> 14);

    f->w2 = f->w1;
    f->w1 = w0;

    return (int16_t)(y >> 14);
}
```

### Cascade Second-Order Sections
- Factor high-order filter into biquads
- Order sections to minimize signal growth
- Use double-precision states for narrow-band filters

---

## Fast Fourier Transform (FFT)

### Radix-2 Butterfly
```
A' = A + W × B
B' = A - W × B
```
Where W = twiddle factor = e^(-j×2π×k/N)

### Fixed-Point FFT
```c
void butterfly_q15(int16_t* ar, int16_t* ai,
                   int16_t* br, int16_t* bi,
                   int16_t wr, int16_t wi) {
    // Complex multiply: (br + j×bi) × (wr + j×wi)
    int32_t tr = (int32_t)br * wr - (int32_t)bi * wi;
    int32_t ti = (int32_t)br * wi + (int32_t)bi * wr;

    // Scale and round
    int16_t tr16 = (int16_t)((tr + 0x4000) >> 15);
    int16_t ti16 = (int16_t)((ti + 0x4000) >> 15);

    // Butterfly
    int16_t ar_new = *ar + tr16;
    int16_t ai_new = *ai + ti16;
    *br = *ar - tr16;
    *bi = *ai - ti16;
    *ar = ar_new;
    *ai = ai_new;
}
```

### Block Floating-Point FFT
Scale at each stage to prevent overflow:
```c
void fft_stage_scale(complex_q15* data, int N) {
    int16_t max_val = find_max_abs(data, N);
    int shift = 0;
    if (max_val > 0x4000) {
        shift = 1;  // Divide by 2
        scale_array(data, N, shift);
    }
    // Track total scaling for final result
}
```

### Twiddle Factor Tables
Pre-computed for each FFT size:
```c
// Q15 twiddle factors for N=256
const int16_t twiddle_real[128] = { /* cos values */ };
const int16_t twiddle_imag[128] = { /* -sin values */ };
```

---

## Resampling

### Upsampling (Interpolation)
1. Insert zeros between samples
2. Apply lowpass filter

### Downsampling (Decimation)
1. Apply lowpass anti-aliasing filter
2. Discard samples

### Polyphase Implementation
More efficient than direct filtering:
```c
// Polyphase interpolation by factor L
for (int phase = 0; phase < L; phase++) {
    output[phase] = fir_q15(input, &coeffs[phase * tap_stride], taps/L);
}
```

### Sample Rate Conversion
For ratio M/L:
1. Upsample by L
2. Downsample by M
3. Polyphase structure combines both

---

## Oscillators

### Direct Digital Synthesis (DDS)
```c
uint32_t phase_acc = 0;
uint32_t phase_inc = (uint32_t)(freq / fs * (1ULL << 32));

int16_t dds_sample(void) {
    phase_acc += phase_inc;
    uint16_t index = phase_acc >> 20;  // 12-bit index
    return sin_table_q15[index];
}
```

### CORDIC Oscillator
```c
void cordic_osc(int16_t* sin_out, int16_t* cos_out, int16_t angle) {
    int16_t x = CORDIC_K;  // 1/K ≈ 0.6073
    int16_t y = 0;
    int16_t z = angle;

    for (int i = 0; i < 15; i++) {
        cordic_iteration(&x, &y, &z, i);
    }

    *cos_out = x;
    *sin_out = y;
}
```

---

## Common DSP Building Blocks

### Multiply-Accumulate (MAC)
Core operation for DSP:
```c
int32_t mac_q15(int32_t acc, int16_t a, int16_t b) {
    return acc + (int32_t)a * b;
}
```

### Saturating Add
```c
int16_t sat_add_q15(int16_t a, int16_t b) {
    int32_t sum = (int32_t)a + b;
    if (sum > 32767) return 32767;
    if (sum < -32768) return -32768;
    return (int16_t)sum;
}
```

### Absolute Value
```c
int16_t abs_q15(int16_t x) {
    if (x == -32768) return 32767;  // Prevent overflow
    return (x < 0) ? -x : x;
}
```

### RMS Calculation
```c
int16_t rms_q15(int16_t* data, int N) {
    int32_t sum_sq = 0;
    for (int i = 0; i < N; i++) {
        sum_sq += (int32_t)data[i] * data[i] >> 15;
    }
    int32_t mean_sq = sum_sq / N;
    return sqrt_q15((int16_t)mean_sq);
}
```

---

## CMSIS-DSP Reference

ARM's CMSIS-DSP provides optimized implementations:

| Function | Description |
|----------|-------------|
| arm_fir_q15 | Q15 FIR filter |
| arm_iir_q15 | Q15 IIR filter |
| arm_cfft_q15 | Complex FFT Q15 |
| arm_rfft_q15 | Real FFT Q15 |
| arm_sin_q15 | Sine lookup |
| arm_cos_q15 | Cosine lookup |
| arm_sqrt_q15 | Square root |

---

## Performance Considerations

### Loop Unrolling
```c
// Process 4 samples per iteration
for (int i = 0; i < N; i += 4) {
    acc += (int32_t)x[i+0] * h[i+0];
    acc += (int32_t)x[i+1] * h[i+1];
    acc += (int32_t)x[i+2] * h[i+2];
    acc += (int32_t)x[i+3] * h[i+3];
}
```

### Circular Buffers
For delay lines:
```c
int16_t circular_buffer[SIZE];
int write_idx = 0;

void push(int16_t sample) {
    circular_buffer[write_idx] = sample;
    write_idx = (write_idx + 1) & (SIZE - 1);  // Assumes power of 2
}
```

---

## References

- [The Scientist and Engineer's Guide to DSP](https://www.dspguide.com/)
- [CMSIS-DSP Documentation](https://arm-software.github.io/CMSIS-DSP/)
- [TI Application Notes on Fixed-Point DSP](https://www.ti.com/lit/an/spra948/spra948.pdf)
