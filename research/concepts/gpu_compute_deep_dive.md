# GPU Compute Deep Dive: OpenCL, Vulkan, CUDA

## GPU Architecture Overview

### Memory Hierarchy
1. **Global Memory**: Large, high latency (~400-600 cycles)
2. **Shared/Local Memory**: Per-workgroup, low latency (~10-20 cycles)
3. **Registers**: Per-thread, fastest
4. **Constant Memory**: Cached, read-only

### Execution Model
- **Workgroups/Thread Blocks**: Groups of threads
- **Wavefronts/Warps**: Execution units (32-64 threads)
- **SIMT**: Single Instruction, Multiple Threads

---

## OpenCL Fixed-Point

### Supported Integer Types
- `char`, `short`, `int`, `long` (8/16/32/64-bit)
- Vector types: `int2`, `int4`, `int8`, `int16`, `short16`, etc.
- Saturating built-ins

### Saturating Operations
```opencl
short4 a, b;
short4 c = add_sat(a, b);  // Saturating add
short4 d = sub_sat(a, b);  // Saturating subtract
short4 e = mul_hi(a, b);   // High bits of product
```

### Q15 Implementation
```opencl
typedef short q15_t;
typedef int q31_t;

#define Q15_FRAC_BITS 15
#define Q15_ONE (1 << Q15_FRAC_BITS)

q15_t q15_mul(q15_t a, q15_t b) {
    q31_t product = (q31_t)a * (q31_t)b;
    return (q15_t)(product >> Q15_FRAC_BITS);
}

q15_t q15_mul_sat(q15_t a, q15_t b) {
    q31_t product = (q31_t)a * (q31_t)b;
    q31_t result = product >> Q15_FRAC_BITS;
    return clamp(result, (q31_t)SHRT_MIN, (q31_t)SHRT_MAX);
}
```

### Vector Processing
```opencl
__kernel void q15_vector_add(
    __global const short8* a,
    __global const short8* b,
    __global short8* c,
    int n
) {
    int gid = get_global_id(0);
    if (gid < n / 8) {
        c[gid] = add_sat(a[gid], b[gid]);
    }
}
```

### Memory Optimization
```opencl
// Use local memory for reused data
__kernel void q15_filter(
    __global const short* input,
    __global short* output,
    __constant short* coeffs,
    int n
) {
    __local short cache[256];
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    // Cooperative load to local memory
    cache[lid] = input[gid];
    barrier(CLK_LOCAL_MEM_FENCE);

    // Compute using local memory
    // ...
}
```

---

## Vulkan Compute Shaders

### Advantages over OpenCL
- Better integration with graphics pipelines
- Explicit synchronization (more control)
- SPIR-V intermediate representation
- Comparable performance (within 10%)

### Memory Management
```
// Vulkan requires explicit memory allocation
VkBuffer buffer;
VkDeviceMemory memory;
vkCreateBuffer(..., &buffer);
vkAllocateMemory(..., &memory);
vkBindBufferMemory(device, buffer, memory, 0);
```

### GLSL Compute Shader
```glsl
#version 450

layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer InputA { int dataA[]; };
layout(std430, binding = 1) readonly buffer InputB { int dataB[]; };
layout(std430, binding = 2) writeonly buffer Output { int result[]; };

uniform int n;

// Q16.16 saturating add
int q16_16_add_sat(int a, int b) {
    int sum = a + b;
    // Overflow detection: if signs match and result sign differs
    int overflow = ((a ^ sum) & (b ^ sum)) >> 31;
    int sat_val = (a >> 31) ^ 0x7FFFFFFF;
    return mix(sum, sat_val, overflow != 0);
}

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx < n) {
        result[idx] = q16_16_add_sat(dataA[idx], dataB[idx]);
    }
}
```

### Vulkan vs OpenCL Synchronization
- Vulkan: Command buffers, explicit barriers
- OpenCL: Events, implicit barriers
- Vulkan can eliminate kernel launch overhead via command buffers

---

## CUDA Considerations

### Integer Intrinsics
```cuda
// Saturating operations
int __vadd_sat(int a, int b);      // 32-bit sat add
short2 __vaddss2(short2 a, short2 b);  // Packed Q15 sat add
short4 __vaddss4(short4 a, short4 b);  // Packed Q7 sat add

// Q15 multiply with saturation
__device__ short q15_mul_cuda(short a, short b) {
    int prod = __mulhi(a, b) << 1;  // Q15 * Q15 >> 15
    return __saturatef(prod);
}
```

### PTX Assembly
```
// Direct PTX for custom operations
asm("mad.sat.s16 %0, %1, %2, %3;" : "=h"(result) : "h"(a), "h"(b), "h"(c));
```

### Licensing Note
CUDA is NVIDIA-proprietary. For libfixp:
- Primary: OpenCL (portable)
- Future: Vulkan compute
- Optional: CUDA backend (separate license)

---

## Performance Considerations

### Integer vs Float on GPU
- Modern GPUs optimize for FP32
- Integer throughput often lower
- Exception: Tensor cores (INT8/INT4)

### When Fixed-Point Wins on GPU
1. **Memory-bound kernels**: Smaller types = faster transfer
2. **FPGA targets**: Intel/Xilinx OpenCL SDKs
3. **Determinism**: Exact reproducibility needed
4. **Tensor operations**: INT8 quantized inference

### When Floating-Point Wins
1. **Compute-bound on consumer GPUs**
2. **Complex transcendentals**
3. **Wide dynamic range**

---

## Memory Bandwidth Optimization

### Type Packing
```opencl
// Pack 4 Q15 values into 64 bits
typedef ulong q15x4_t;

q15x4_t pack_q15x4(short a, short b, short c, short d) {
    return ((ulong)a) | ((ulong)b << 16) |
           ((ulong)c << 32) | ((ulong)d << 48);
}
```

### Coalesced Access
```opencl
// Good: Sequential access
result[gid] = add_sat(a[gid], b[gid]);

// Bad: Strided access
result[gid * stride] = add_sat(a[gid * stride], b[gid * stride]);
```

### Pinned Memory
```c
// OpenCL: Use CL_MEM_ALLOC_HOST_PTR
cl_mem buffer = clCreateBuffer(context,
    CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
    size, NULL, &err);

// Map for zero-copy access
void* ptr = clEnqueueMapBuffer(queue, buffer, CL_TRUE,
    CL_MAP_WRITE, 0, size, 0, NULL, NULL, &err);
```

---

## Fixed-Point Transcendentals on GPU

### CORDIC for GPU
```opencl
// GPU-friendly CORDIC (no branching)
void cordic_sincos_gpu(int angle, int* sin_out, int* cos_out) {
    int x = CORDIC_SCALE;  // Pre-scaled 1/K
    int y = 0;
    int z = angle;

    for (int i = 0; i < CORDIC_ITERATIONS; i++) {
        int sigma = (z >= 0) ? 1 : -1;
        int x_new = x - sigma * (y >> i);
        int y_new = y + sigma * (x >> i);
        z = z - sigma * atan_table[i];
        x = x_new;
        y = y_new;
    }

    *cos_out = x;
    *sin_out = y;
}
```

### Polynomial on GPU
```opencl
// Horner's method (efficient for GPU)
q16_16 sin_poly(q16_16 x) {
    // Coefficients for range-reduced sin
    const q16_16 c1 = 0x0000FFFF;  // ~1.0
    const q16_16 c3 = 0xFFFFD556;  // ~-1/6
    const q16_16 c5 = 0x00000222;  // ~1/120

    q16_16 x2 = q16_16_mul(x, x);
    q16_16 result = c5;
    result = q16_16_mul(result, x2) + c3;
    result = q16_16_mul(result, x2) + c1;
    return q16_16_mul(result, x);
}
```

---

## libfixp GPU Strategy

### OpenCL Header Generation
```c
// Generate from C++ templates
const char* kernel_source = R"(
#ifndef FRAC_BITS
#define FRAC_BITS 16
#endif

typedef int q_t;

q_t q_add_sat(q_t a, q_t b) { ... }
q_t q_mul(q_t a, q_t b) { ... }
// etc.
)";
```

### Runtime Configuration
```c
// Set Q format at kernel compile time
char options[256];
sprintf(options, "-DFRAC_BITS=%d -DWORD_SIZE=%d",
        frac_bits, word_size);
clBuildProgram(program, 1, &device, options, NULL, NULL);
```

### Unified API
```cpp
namespace libfixp::gpu {
    template<typename QType>
    class Context {
        void* platform_handle;  // cl_context or VkDevice

    public:
        Buffer<QType> create_buffer(size_t n);
        void add(Buffer<QType>& out,
                 const Buffer<QType>& a,
                 const Buffer<QType>& b);
    };
}
```

---

## References

- [OpenCL 3.0 Specification](https://www.khronos.org/opencl/)
- [Vulkan Compute Tutorial](https://vulkan-tutorial.com/Compute_Shader)
- [Intel OpenCL Optimization Guide](https://www.intel.com/content/www/us/en/docs/programmable/683521/17-1/)
- [CUDA C Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)
