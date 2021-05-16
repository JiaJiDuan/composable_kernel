#ifndef CK_AMD_XDLOPS_HPP
#define CK_AMD_XDLOPS_HPP

#include "float_type.hpp"

namespace ck {

struct c_vec32_4_t
{
    union VecType
    {
        struct
        {
            float32_t x;
            float32_t y;
            float32_t z;
            float32_t w;
        } s;
        float n[128];
    };

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x = 0;
        c.s.y = 0;
        c.s.z = 0;
        c.s.w = 0;
        return c;
    }
};

struct c_vec32_2_t
{
    union VecType
    {
        struct
        {
            float32_t x;
            float32_t y;
        } s;
        float n[64];
    } l;

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x = 0;
        c.s.y = 0;
        return c;
    }
};

struct c_vec32_2_2_t
{
    union VecType
    {
        struct
        {
            c_vec32_2_t x;
            c_vec32_2_t y;
        } s;
        float n[128];
    };

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x.l.s.x = 0;
        c.s.x.l.s.y = 0;
        c.s.y.l.s.x = 0;
        c.s.y.l.s.y = 0;
        return c;
    }
};

struct c_vec32_1_t
{
    union VecType
    {
        struct
        {
            float32_t x;
        } s;
        float n[32];
    };

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x = 0;
        return c;
    }
};

struct c_vec16_1_t
{
    union VecType
    {
        struct
        {
            float16_t x;
        } s;
        float n[16];
    };

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x = 0;
        return c;
    }
};

struct c_vec4_2_t
{
    union VecType
    {
        struct
        {
            float4_t x;
            float4_t y;
        } s;
        float n[8];
    };

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x = 0;
        c.s.y = 0;
        return c;
    }
};

struct c_vec4_1_t
{
    union VecType
    {
        struct
        {
            float4_t x;
        } s;
        float n[4];
    };

    __host__ __device__ static VecType CreateVecZero()
    {
        VecType c;
        c.s.x = 0;
        return c;
    }
};

// A, B, C, cbsz, abid, blgp
extern "C" __device__ float32_t llvm_intrin_amdgcn_mfma_f32_32x32x1f32(
    float, float, float32_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x1f32");

extern "C" __device__ float16_t llvm_intrin_amdgcn_mfma_f32_32x32x2f32(
    float, float, float16_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x2f32");

extern "C" __device__ float4_t llvm_intrin_amdgcn_mfma_f32_16x16x4f32(
    float, float, float4_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.16x16x4f32");

extern "C" __device__ float16_t llvm_intrin_amdgcn_mfma_f32_16x16x1f32(
    float, float, float16_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.16x16x1f32");

extern "C" __device__ float4_t llvm_intrin_amdgcn_mfma_f32_4x4x1f32(
    float, float, float4_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.4x4x1f32");

extern "C" __device__ float32_t llvm_intrin_amdgcn_mfma_f32_32x32x4f16(
    half4_t, half4_t, float32_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x4f16");

extern "C" __device__ float16_t llvm_intrin_amdgcn_mfma_f32_32x32x8f16(
    half4_t, half4_t, float16_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x8f16");

extern "C" __device__ float4_t llvm_intrin_amdgcn_mfma_f32_16x16x16f16(
    half4_t, half4_t, float4_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.16x16x16f16");

extern "C" __device__ float16_t llvm_intrin_amdgcn_mfma_f32_16x16x4f16(
    half4_t, half4_t, float16_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.16x16x4f16");

extern "C" __device__ float4_t llvm_intrin_amdgcn_mfma_f32_4x4x4f16(
    half4_t, half4_t, float4_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.4x4x4f16");

extern "C" __device__ float32_t llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(
    ushort2_t, ushort2_t, float32_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x2bf16");

extern "C" __device__ float16_t llvm_intrin_amdgcn_mfma_f32_32x32x4bf16(
    ushort2_t, ushort2_t, float16_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x4bf16");

extern "C" __device__ float4_t llvm_intrin_amdgcn_mfma_f32_16x16x8bf16(
    ushort2_t, ushort2_t, float4_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.16x16x8bf16");

extern "C" __device__ float16_t llvm_intrin_amdgcn_mfma_f32_16x16x2bf16(
    ushort2_t, ushort2_t, float16_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.16x16x2bf16");

extern "C" __device__ float4_t llvm_intrin_amdgcn_mfma_f32_4x4x2bf16(
    ushort2_t, ushort2_t, float4_t, int, int, int) __asm("llvm.amdgcn.mfma.f32.4x4x2bf16");

template <index_t MPerWave, index_t NPerWave>
struct intrin_mfma_f32_32x32x1f32;

// template <index_t AStride, index_t BStride>
// struct intrin_mfma_f32_32x32x1f32<128, 64, AStride, BStride>
//{
//__device__ static c_vec32_4_t::VecType
// run(const float* reg_a, const float* reg_b, c_vec32_4_t::VecType reg_c)
//{
// reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
// reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

// reg_c.s.z =
// llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[AStride], reg_b[0], reg_c.s.z, 1, 0, 0);
// reg_c.s.w =
// llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[AStride], reg_b[0], reg_c.s.w, 1, 1, 0);

// return reg_c;
//}
//};

// template <index_t AStride, index_t BStride>
// struct intrin_mfma_f32_32x32x1f32<64, 128, AStride, BStride>
//{
//__device__ static c_vec32_4_t::VecType
// run(const float* reg_a, const float* reg_b, c_vec32_4_t::VecType reg_c)
//{
// reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
// reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

// reg_c.s.z =
// llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[BStride], reg_c.s.z, 1, 0, 0);
// reg_c.s.w =
// llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[BStride], reg_c.s.w, 1, 1, 0);

// return reg_c;
//}
//};

template <>
struct intrin_mfma_f32_32x32x1f32<64, 64>
{
    __device__ static void
    run(const float& reg_a, const float& reg_b, vector_type<float, 64>& reg_c)
    {
        reg_c.template AsType<float32_t>()(Number<0>{}) = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(
            reg_a, reg_b, reg_c.template AsType<float32_t>()[Number<0>{}], 1, 0, 0);
        reg_c.template AsType<float32_t>()(Number<1>{}) = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(
            reg_a, reg_b, reg_c.template AsType<float32_t>()[Number<1>{}], 1, 1, 0);
    }
};

// template <index_t AStride, index_t BStride>
// struct intrin_mfma_f32_32x32x1f32<64, 32, AStride, BStride>
//{
//__device__ static c_vec32_1_t::VecType
// run(const float* reg_a, const float* reg_b, c_vec32_1_t::VecType reg_c)
//{
// reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 1);
// return reg_c;
//}
//};

// template <index_t AStride, index_t BStride>
// struct intrin_mfma_f32_32x32x1f32<32, 64, AStride, BStride>
//{
//__device__ static c_vec32_1_t::VecType
// run(const float* reg_a, const float* reg_b, c_vec32_1_t::VecType reg_c)
//{
// reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x1f32(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
// return reg_c;
//}
//};

__device__ c_vec16_1_t::VecType
intrin_mfma_f32_32x32x2f32(const float* reg_a, const float* reg_b, c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x2f32(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 0);
    return reg_c;
}

__device__ c_vec4_1_t::VecType
intrin_mfma_f32_16x16x4f32(const float* reg_a, const float* reg_b, c_vec4_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x4f32(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 0);
    return reg_c;
}

template <index_t MPerWave, index_t NPerWave>
__device__ c_vec16_1_t::VecType
intrin_mfma_f32_16x16x1f32(const float* reg_a, const float* reg_b, c_vec16_1_t::VecType reg_c);

template <>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x1f32<16, 64>(const float* reg_a,
                                                                   const float* reg_b,
                                                                   c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x1f32(reg_a[0], reg_b[0], reg_c.s.x, 2, 0, 0);
    return reg_c;
}

template <>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x1f32<64, 16>(const float* reg_a,
                                                                   const float* reg_b,
                                                                   c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x1f32(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 4);
    return reg_c;
}

template <index_t MPerWave, index_t NPerWave>
struct intrin_mfma_f32_4x4x1f32;

template <>
struct intrin_mfma_f32_4x4x1f32<4, 64>
{
    __device__ static c_vec4_1_t::VecType
    run(const float* reg_a, const float* reg_b, c_vec4_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_4x4x1f32(reg_a[0], reg_b[0], reg_c.s.x, 4, 0, 0);
        return reg_c;
    }
};

template <>
struct intrin_mfma_f32_4x4x1f32<8, 64>
{
    __device__ static c_vec4_2_t::VecType
    run(const float* reg_a, const float* reg_b, c_vec4_2_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_4x4x1f32(reg_a[0], reg_b[0], reg_c.s.x, 4, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_4x4x1f32(reg_a[0], reg_b[0], reg_c.s.y, 4, 1, 0);
        return reg_c;
    }
};

template <index_t MPerWave, index_t NPerWave, index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x4f16;

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x4f16<128, 64, AStride, BStride>
{
    __device__ static c_vec32_4_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec32_4_t::VecType reg_c)
    {

        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

        reg_c.s.z =
            llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[AStride], reg_b[0], reg_c.s.z, 1, 0, 0);
        reg_c.s.w =
            llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[AStride], reg_b[0], reg_c.s.w, 1, 1, 0);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x4f16<64, 128, AStride, BStride>
{
    __device__ static c_vec32_4_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec32_4_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

        reg_c.s.z =
            llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[BStride], reg_c.s.z, 1, 0, 0);
        reg_c.s.w =
            llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[BStride], reg_c.s.w, 1, 1, 0);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x4f16<64, 64, AStride, BStride>
{
    __device__ static c_vec32_2_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec32_2_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x4f16<64, 32, AStride, BStride>
{
    __device__ static c_vec32_1_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec32_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 1);
        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x4f16<32, 64, AStride, BStride>
{
    __device__ static c_vec32_1_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec32_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x4f16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        return reg_c;
    }
};

__device__ c_vec16_1_t::VecType
intrin_mfma_f32_32x32x8f16(const half4_t* reg_a, const half4_t* reg_b, c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x8f16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 0);
    return reg_c;
}

__device__ c_vec4_1_t::VecType
intrin_mfma_f32_16x16x16f16(const half4_t* reg_a, const half4_t* reg_b, c_vec4_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x16f16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 0);
    return reg_c;
}

template <index_t MPerWave, index_t NPerWave>
__device__ c_vec16_1_t::VecType
intrin_mfma_f32_16x16x4f16(const half4_t* reg_a, const half4_t* reg_b, c_vec16_1_t::VecType reg_c);

template <>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x4f16<16, 64>(const half4_t* reg_a,
                                                                   const half4_t* reg_b,
                                                                   c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x4f16(reg_a[0], reg_b[0], reg_c.s.x, 2, 0, 0);
    return reg_c;
}

template <>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x4f16<64, 16>(const half4_t* reg_a,
                                                                   const half4_t* reg_b,
                                                                   c_vec16_1_t::VecType reg_c)

{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x4f16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 4);
    return reg_c;
}

template <index_t MPerWave, index_t NPerWave>
struct intrin_mfma_f32_4x4x4f16;

template <>
struct intrin_mfma_f32_4x4x4f16<4, 64>
{
    __device__ static c_vec4_1_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec4_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_4x4x4f16(reg_a[0], reg_b[0], reg_c.s.x, 4, 0, 0);
        return reg_c;
    }
};

template <>
struct intrin_mfma_f32_4x4x4f16<8, 64>
{
    __device__ static c_vec4_2_t::VecType
    run(const half4_t* reg_a, const half4_t* reg_b, c_vec4_2_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_4x4x4f16(reg_a[0], reg_b[0], reg_c.s.x, 4, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_4x4x4f16(reg_a[0], reg_b[0], reg_c.s.y, 4, 1, 0);

        return reg_c;
    }
};

template <index_t MPerWave, index_t NPerWave, index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x2bf16;

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x2bf16<128, 64, AStride, BStride>
{
    __device__ static c_vec32_4_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec32_4_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

        reg_c.s.z =
            llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[AStride], reg_b[0], reg_c.s.z, 1, 0, 0);
        reg_c.s.w =
            llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[AStride], reg_b[0], reg_c.s.w, 1, 1, 0);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x2bf16<64, 128, AStride, BStride>
{
    __device__ static c_vec32_4_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec32_4_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

        reg_c.s.z =
            llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[BStride], reg_c.s.z, 1, 0, 0);
        reg_c.s.w =
            llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[BStride], reg_c.s.w, 1, 1, 0);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x2bf16<64, 64, AStride, BStride>
{
    __device__ static c_vec32_2_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec32_2_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.y, 1, 1, 0);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x2bf16<64, 32, AStride, BStride>
{
    __device__ static c_vec32_1_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec32_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 1);

        return reg_c;
    }
};

template <index_t AStride, index_t BStride>
struct intrin_mfma_f32_32x32x2bf16<32, 64, AStride, BStride>
{
    __device__ static c_vec32_1_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec32_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 1, 0, 0);
        return reg_c;
    }
};

__device__ c_vec16_1_t::VecType intrin_mfma_f32_32x32x4bf16(const ushort2_t* reg_a,
                                                            const ushort2_t* reg_b,
                                                            c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_32x32x4bf16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 0);
    return reg_c;
}

__device__ c_vec4_1_t::VecType intrin_mfma_f32_16x16x8bf16(const ushort2_t* reg_a,
                                                           const ushort2_t* reg_b,
                                                           c_vec4_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x8bf16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 0);
    return reg_c;
}

template <index_t MPerWave, index_t NPerWave>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x2bf16(const ushort2_t* reg_a,
                                                            const ushort2_t* reg_b,
                                                            c_vec16_1_t::VecType reg_c);

template <>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x2bf16<16, 64>(const ushort2_t* reg_a,
                                                                    const ushort2_t* reg_b,
                                                                    c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 2, 0, 0);
    return reg_c;
}

template <>
__device__ c_vec16_1_t::VecType intrin_mfma_f32_16x16x2bf16<64, 16>(const ushort2_t* reg_a,
                                                                    const ushort2_t* reg_b,
                                                                    c_vec16_1_t::VecType reg_c)
{
    reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_16x16x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 0, 0, 4);
    return reg_c;
}

template <index_t MPerWave, index_t NPerWave>
struct intrin_mfma_f32_4x4x2bf16;

template <>
struct intrin_mfma_f32_4x4x2bf16<4, 64>
{
    __device__ static c_vec4_1_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec4_1_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_4x4x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 4, 0, 0);
        return reg_c;
    }
};

template <>
struct intrin_mfma_f32_4x4x2bf16<8, 64>
{
    __device__ static c_vec4_2_t::VecType
    run(const ushort2_t* reg_a, const ushort2_t* reg_b, c_vec4_2_t::VecType reg_c)
    {
        reg_c.s.x = llvm_intrin_amdgcn_mfma_f32_4x4x2bf16(reg_a[0], reg_b[0], reg_c.s.x, 4, 0, 0);
        reg_c.s.y = llvm_intrin_amdgcn_mfma_f32_4x4x2bf16(reg_a[0], reg_b[0], reg_c.s.y, 4, 1, 0);
        return reg_c;
    }
};
} // namespace ck
#endif
