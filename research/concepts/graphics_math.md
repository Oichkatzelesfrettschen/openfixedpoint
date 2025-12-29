# Graphics Math in Fixed-Point

## Coordinate Systems

### Screen Coordinates
Typically integer pixels, but sub-pixel precision useful for anti-aliasing:
```
Q16.16: Range ±32K pixels, 1/65536 sub-pixel precision
Q12.4:  Range ±2K pixels, 1/16 sub-pixel precision (faster)
```

### Normalized Device Coordinates
Values in [-1, 1]:
```
Q1.15: Full 16-bit precision in [-1, 1)
Q1.31: Maximum precision for transformations
```

---

## Vectors

### 2D Vector (Fixed-Point)
```c
typedef struct {
    q16_16 x, y;
} vec2_fp;

q16_16 vec2_dot(vec2_fp a, vec2_fp b) {
    q32_32 dot = (q32_32)a.x * b.x + (q32_32)a.y * b.y;
    return (q16_16)(dot >> 16);
}

q16_16 vec2_length(vec2_fp v) {
    q32_32 len_sq = (q32_32)v.x * v.x + (q32_32)v.y * v.y;
    return sqrt_q16_16((q16_16)(len_sq >> 16));
}
```

### 3D Vector
```c
typedef struct {
    q16_16 x, y, z;
} vec3_fp;

vec3_fp vec3_cross(vec3_fp a, vec3_fp b) {
    vec3_fp result;
    result.x = q16_16_mul(a.y, b.z) - q16_16_mul(a.z, b.y);
    result.y = q16_16_mul(a.z, b.x) - q16_16_mul(a.x, b.z);
    result.z = q16_16_mul(a.x, b.y) - q16_16_mul(a.y, b.x);
    return result;
}
```

---

## Matrices

### 4x4 Transformation Matrix
```c
typedef struct {
    q16_16 m[4][4];
} mat4_fp;

vec3_fp mat4_transform_point(mat4_fp* m, vec3_fp p) {
    vec3_fp result;
    // Homogeneous transform (w = 1)
    result.x = q16_16_mul(m->m[0][0], p.x)
             + q16_16_mul(m->m[0][1], p.y)
             + q16_16_mul(m->m[0][2], p.z)
             + m->m[0][3];
    result.y = q16_16_mul(m->m[1][0], p.x)
             + q16_16_mul(m->m[1][1], p.y)
             + q16_16_mul(m->m[1][2], p.z)
             + m->m[1][3];
    result.z = q16_16_mul(m->m[2][0], p.x)
             + q16_16_mul(m->m[2][1], p.y)
             + q16_16_mul(m->m[2][2], p.z)
             + m->m[2][3];
    return result;
}
```

### Matrix Multiplication Precision
```
3x3 matrix multiply: 9 MACs per element
4x4 matrix multiply: 16 MACs per element

For Q16.16: Each MAC produces Q32.32
            Need 64-bit accumulator
            Final result rounds to Q16.16
```

---

## Quaternions

### Representation
```
q = w + xi + yj + zk
|q| = 1 for rotation quaternions (unit quaternion)
```

### Fixed-Point Quaternion
```c
typedef struct {
    q2_14 w, x, y, z;  // Q2.14 for range [-2, 2) with 14 bits precision
} quat_fp;

// Alternatively, for unit quaternions:
typedef struct {
    q1_15 w, x, y, z;  // Q1.15 for range [-1, 1)
} unit_quat_fp;
```

### Quaternion Multiplication
```c
quat_fp quat_mul(quat_fp a, quat_fp b) {
    quat_fp r;
    // (a.w + a.xi + a.yj + a.zk)(b.w + b.xi + b.yj + b.zk)
    r.w = q2_14_mul(a.w, b.w) - q2_14_mul(a.x, b.x)
        - q2_14_mul(a.y, b.y) - q2_14_mul(a.z, b.z);
    r.x = q2_14_mul(a.w, b.x) + q2_14_mul(a.x, b.w)
        + q2_14_mul(a.y, b.z) - q2_14_mul(a.z, b.y);
    r.y = q2_14_mul(a.w, b.y) - q2_14_mul(a.x, b.z)
        + q2_14_mul(a.y, b.w) + q2_14_mul(a.z, b.x);
    r.z = q2_14_mul(a.w, b.z) + q2_14_mul(a.x, b.y)
        - q2_14_mul(a.y, b.x) + q2_14_mul(a.z, b.w);
    return r;
}
```

### Rotate Vector by Quaternion
```
v' = q × v × q*  (quaternion sandwich)
```

More efficient form:
```c
vec3_fp quat_rotate_vec(quat_fp q, vec3_fp v) {
    // t = 2 × (q.xyz × v)
    vec3_fp qv = {q.x, q.y, q.z};
    vec3_fp t = vec3_cross(qv, v);
    t.x <<= 1; t.y <<= 1; t.z <<= 1;

    // v' = v + q.w × t + (q.xyz × t)
    vec3_fp result;
    vec3_fp qt = vec3_cross(qv, t);
    result.x = v.x + q2_14_mul(q.w, t.x) + qt.x;
    result.y = v.y + q2_14_mul(q.w, t.y) + qt.y;
    result.z = v.z + q2_14_mul(q.w, t.z) + qt.z;
    return result;
}
```

### Quaternion vs Matrix for Rotations

| Aspect | Quaternion | Matrix (3x3) |
|--------|------------|--------------|
| Storage | 4 values | 9 values |
| Compose | 16 muls + 12 adds | 27 muls + 18 adds |
| Transform point | 24 muls + 17 adds | 9 muls + 6 adds |
| Interpolation | SLERP (natural) | Complex |
| Gimbal lock | No | No |
| Renormalize | Easy (4 values) | Harder (orthogonalize) |

### SLERP (Spherical Linear Interpolation)
```c
quat_fp slerp(quat_fp a, quat_fp b, q16_16 t) {
    q16_16 dot = quat_dot(a, b);

    // If dot < 0, negate one to take shorter path
    if (dot < 0) {
        b.w = -b.w; b.x = -b.x; b.y = -b.y; b.z = -b.z;
        dot = -dot;
    }

    // If nearly parallel, use linear interpolation
    if (dot > 0xFFF0) {  // ~0.9999
        return quat_lerp(a, b, t);
    }

    q16_16 theta = acos_q16_16(dot);
    q16_16 sin_theta = sin_q16_16(theta);

    q16_16 t1 = sin_q16_16(q16_16_mul(Q16_16_ONE - t, theta));
    q16_16 t2 = sin_q16_16(q16_16_mul(t, theta));

    // Blend and normalize
    quat_fp result;
    result.w = (t1 * a.w + t2 * b.w) / sin_theta;
    // ... similar for x, y, z
    return quat_normalize(result);
}
```

---

## Euler Angles

### Advantages for Fixed-Point
- Simple quantization (just angles)
- Easy to pack into small bit widths
- Uniform precision across range

### Conversion to Quaternion
```c
quat_fp euler_to_quat(q16_16 yaw, q16_16 pitch, q16_16 roll) {
    // Half angles
    q16_16 cy = cos_q16_16(yaw >> 1);
    q16_16 sy = sin_q16_16(yaw >> 1);
    q16_16 cp = cos_q16_16(pitch >> 1);
    q16_16 sp = sin_q16_16(pitch >> 1);
    q16_16 cr = cos_q16_16(roll >> 1);
    q16_16 sr = sin_q16_16(roll >> 1);

    quat_fp q;
    q.w = cr*cp*cy + sr*sp*sy;
    q.x = sr*cp*cy - cr*sp*sy;
    q.y = cr*sp*cy + sr*cp*sy;
    q.z = cr*cp*sy - sr*sp*cy;
    return q;
}
```

---

## Fixed-Point Trigonometry for Graphics

### Common Requirements
- sin/cos for rotations
- atan2 for angle from vector
- acos for SLERP

### Accuracy Needs
- 16-bit angles: 360°/65536 ≈ 0.0055° resolution
- Usually sufficient for graphics

### Lookup Table Optimization
```c
// 256-entry quarter-wave sine table
const q1_15 sin_table[256] = { ... };

q1_15 sin_fast(uint16_t angle) {
    uint8_t quadrant = angle >> 14;  // 0-3
    uint8_t index = (angle >> 6) & 0xFF;

    q1_15 value;
    switch (quadrant) {
        case 0: value = sin_table[index]; break;
        case 1: value = sin_table[255 - index]; break;
        case 2: value = -sin_table[index]; break;
        case 3: value = -sin_table[255 - index]; break;
    }
    return value;
}
```

---

## Precision Recommendations

| Use Case | Recommended Format |
|----------|-------------------|
| Pixel coordinates | Q16.16 or Q12.4 |
| UV coordinates | Q0.16 (unsigned) |
| Colors (HDR) | Q8.8 per channel |
| Normals | Q1.15 |
| Quaternion components | Q2.14 or Q1.15 |
| Matrices | Q16.16 |
| Angles | Q16 (full rotation = 65536) |

---

## References

- [Understanding Quaternions](https://www.3dgep.com/understanding-quaternions/)
- [Quaternions and Rotations (Stanford)](https://graphics.stanford.edu/courses/cs348a-17-winter/Papers/quaternion.pdf)
- [3D Math Primer](https://gamemath.com/book/orient.html)
