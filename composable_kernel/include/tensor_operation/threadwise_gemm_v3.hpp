#ifndef CK_THREADWISE_GEMM_V3_HPP
#define CK_THREADWISE_GEMM_V3_HPP

#include "common_header.hpp"
#include "math.hpp"

namespace ck {

#if 0
template <typename Float, typename Desc>
__device__ void threadwise_matrix_set_zero_v3(Desc, Float* __restrict__ p_thread)
{
    static_assert(Desc::IsKnownAtCompileTime(), "wrong! Desc should be known at compile-time");

    constexpr auto I0 = Number<0>{};
    constexpr auto I2 = Number<2>{};
    constexpr auto I3 = Number<3>{};

    constexpr auto desc = Desc{};

    constexpr auto K = desc.GetLength(I0);
    constexpr auto H = desc.GetLength(I2);
    constexpr auto W = desc.GetLength(I3);

    static_for<0, K, 1>{}([&](auto i) {
        static_for<0, H, 1>{}([&](auto j) {
            static_for<0, W, 1>{}([&](auto k) {
                constexpr auto offset = desc.CalculateOffset(make_tuple(i, 0, j, k));

                p_thread[offset] = Float(0);
            });
        });
    });
}
#endif

#if 0
// C[M, N] += transpose(A[K, M]) * B[K, N]
//   Element of matrix can be vectorized data
template <typename FloatA,
          typename FloatB,
          typename FloatC,
          typename ADesc,
          typename BDesc,
          typename CDesc,
          index_t H,
          index_t W,
          typename std::enable_if<ADesc::IsKnownAtCompileTime() && BDesc::IsKnownAtCompileTime() &&
                                      CDesc::IsKnownAtCompileTime(),
                                  bool>::type = false>
struct ThreadwiseGemm_km_kn_mn_v3
{
    __device__ static void Run(const FloatA* p_a, const FloatB* p_b, FloatC* p_c)
    {
        static_assert(ADesc::IsKnownAtCompileTime() && BDesc::IsKnownAtCompileTime() &&
                          CDesc::IsKnownAtCompileTime(),
                      "wrong! Desc should be known at compile-time");

        constexpr auto I0 = Number<0>{};
        constexpr auto I1 = Number<1>{};
        constexpr auto I2 = Number<2>{};
        constexpr auto I3 = Number<3>{};

        constexpr auto E = ADesc{}.GetLength(I0);
        constexpr auto K = ADesc{}.GetLength(I1);

        static_for<0, E, 1>{}([&](auto e) {
            static_for<0, K, 1>{}([&](auto k) {
                constexpr index_t a_offset = ADesc{}.CalculateOffset(make_tuple(e, k));

                if constexpr(H == 2 && W == 2)
                {

                    constexpr index_t b_offset_0 = BDesc{}.CalculateOffset(make_tuple(e, 0, 0, 0));
                    constexpr index_t b_offset_1 = BDesc{}.CalculateOffset(make_tuple(e, 0, 0, 1));
                    constexpr index_t b_offset_2 = BDesc{}.CalculateOffset(make_tuple(e, 0, 1, 0));
                    constexpr index_t b_offset_3 = BDesc{}.CalculateOffset(make_tuple(e, 0, 1, 1));

                    constexpr index_t c_offset_0 = CDesc{}.CalculateOffset(make_tuple(k, 0, 0, 0));
                    constexpr index_t c_offset_1 = CDesc{}.CalculateOffset(make_tuple(k, 0, 0, 1));
                    constexpr index_t c_offset_2 = CDesc{}.CalculateOffset(make_tuple(k, 0, 1, 0));
                    constexpr index_t c_offset_3 = CDesc{}.CalculateOffset(make_tuple(k, 0, 1, 1));

                    amd_assembly_outer_product_1x4(p_a[Number<a_offset>{}],
                                                   p_b[Number<b_offset_0>{}],
                                                   p_b[Number<b_offset_1>{}],
                                                   p_b[Number<b_offset_2>{}],
                                                   p_b[Number<b_offset_3>{}],
                                                   p_c[Number<c_offset_0>{}],
                                                   p_c[Number<c_offset_1>{}],
                                                   p_c[Number<c_offset_2>{}],
                                                   p_c[Number<c_offset_3>{}]);
                }
                else if constexpr(H == 4 && W == 1)
                {

                    constexpr index_t b_offset_0 = BDesc{}.CalculateOffset(make_tuple(e, 0, 0, 0));
                    constexpr index_t b_offset_1 = BDesc{}.CalculateOffset(make_tuple(e, 0, 1, 0));
                    constexpr index_t b_offset_2 = BDesc{}.CalculateOffset(make_tuple(e, 0, 2, 0));
                    constexpr index_t b_offset_3 = BDesc{}.CalculateOffset(make_tuple(e, 0, 3, 0));

                    constexpr index_t c_offset_0 = CDesc{}.CalculateOffset(make_tuple(k, 0, 0, 0));
                    constexpr index_t c_offset_1 = CDesc{}.CalculateOffset(make_tuple(k, 0, 1, 0));
                    constexpr index_t c_offset_2 = CDesc{}.CalculateOffset(make_tuple(k, 0, 2, 0));
                    constexpr index_t c_offset_3 = CDesc{}.CalculateOffset(make_tuple(k, 0, 3, 0));

                    amd_assembly_outer_product_1x4(p_a[Number<a_offset>{}],
                                                   p_b[Number<b_offset_0>{}],
                                                   p_b[Number<b_offset_1>{}],
                                                   p_b[Number<b_offset_2>{}],
                                                   p_b[Number<b_offset_3>{}],
                                                   p_c[Number<c_offset_0>{}],
                                                   p_c[Number<c_offset_1>{}],
                                                   p_c[Number<c_offset_2>{}],
                                                   p_c[Number<c_offset_3>{}]);
                }
                else
                {
                    static_for<0, H, 1>{}([&](auto h) {
                        static_for<0, W, 1>{}([&](auto w) {
                            constexpr index_t b_offset =
                                BDesc{}.CalculateOffset(make_tuple(e, 0, h, w));
                            constexpr index_t c_offset =
                                CDesc{}.CalculateOffset(make_tuple(k, 0, h, w));

#if 0
                            p_c[Number<c_offset>{}] += inner_product_with_conversion<FloatC>{}(p_a[Number<a_offset>{}],
                                                                                               p_b[Number<b_offset>{}]);
#else
                            amd_assembly_inner_product(p_a[Number<a_offset>{}],
                                                       p_b[Number<b_offset>{}],
                                                       p_c[Number<c_offset>{}]);
#endif
                        });
                    });
                }
            });
        });
    }
};
#else
// C[M, N] += transpose(A[K, M]) * B[K, N]
//   Element of matrix can be vectorized data
// Assume:
//   1. ADesc, BDesc, CDesc are known at compile-time
//   2. AOriginIdx, BOriginIdx, COriginIdx are known at compile-time
template <typename FloatA,
          typename FloatB,
          typename FloatC,
          typename ADesc,
          typename BDesc,
          typename CDesc,
          index_t H,
          index_t W,
          typename std::enable_if<ADesc::IsKnownAtCompileTime() && BDesc::IsKnownAtCompileTime() &&
                                      CDesc::IsKnownAtCompileTime(),
                                  bool>::type = false>
struct ThreadwiseGemm_km_kn_mn_v3
{
    template <typename ABuffer,
              typename AOriginIdx,
              typename BBuffer,
              typename BOriginIdx,
              typename CBuffer,
              typename COriginIdx>
    __device__ static void Run(const ABuffer& a_buf,
                               AOriginIdx,
                               const BBuffer& b_buf,
                               BOriginIdx,
                               CBuffer& c_buf,
                               COriginIdx)
    {
        static_assert(ADesc::IsKnownAtCompileTime() && BDesc::IsKnownAtCompileTime() &&
                          CDesc::IsKnownAtCompileTime(),
                      "wrong! Desc should be known at compile-time");

        static_assert(
            is_known_at_compile_time<remove_cv_t<remove_reference_t<AOriginIdx>>>::value &&
                is_known_at_compile_time<remove_cv_t<remove_reference_t<BOriginIdx>>>::value &&
                is_known_at_compile_time<remove_cv_t<remove_reference_t<COriginIdx>>>::value,
            "wrong! AOriginIdx, BOriginIdx, COringinIdx should be known at compile-time");

        constexpr auto I0 = Number<0>{};
        constexpr auto I1 = Number<1>{};
        constexpr auto I2 = Number<2>{};
        constexpr auto I3 = Number<3>{};

        constexpr auto E = ADesc{}.GetLength(I0);
        constexpr auto K = ADesc{}.GetLength(I1);

        constexpr auto a_origin_idx = to_multi_index(AOriginIdx{});
        constexpr auto b_origin_idx = to_multi_index(BOriginIdx{});
        constexpr auto c_origin_idx = to_multi_index(COriginIdx{});

        static_for<0, E, 1>{}([&](auto e) {
            static_for<0, K, 1>{}([&](auto k) {
                constexpr index_t a_offset =
                    ADesc{}.CalculateOffset(a_origin_idx + make_tuple(e, k));

                if constexpr(H == 2 && W == 2)
                {
                    constexpr index_t b_offset_0 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 0, 0));
                    constexpr index_t b_offset_1 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 0, 1));
                    constexpr index_t b_offset_2 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 1, 0));
                    constexpr index_t b_offset_3 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 1, 1));

                    constexpr index_t c_offset_0 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 0, 0));
                    constexpr index_t c_offset_1 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 0, 1));
                    constexpr index_t c_offset_2 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 1, 0));
                    constexpr index_t c_offset_3 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 1, 1));

                    amd_assembly_outer_product_1x4(a_buf[Number<a_offset>{}],
                                                   b_buf[Number<b_offset_0>{}],
                                                   b_buf[Number<b_offset_1>{}],
                                                   b_buf[Number<b_offset_2>{}],
                                                   b_buf[Number<b_offset_3>{}],
                                                   c_buf(Number<c_offset_0>{}),
                                                   c_buf(Number<c_offset_1>{}),
                                                   c_buf(Number<c_offset_2>{}),
                                                   c_buf(Number<c_offset_3>{}));
                }
                else if constexpr(H == 4 && W == 1)
                {
                    constexpr index_t b_offset_0 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 0, 0));
                    constexpr index_t b_offset_1 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 1, 0));
                    constexpr index_t b_offset_2 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 2, 0));
                    constexpr index_t b_offset_3 =
                        BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, 3, 0));

                    constexpr index_t c_offset_0 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 0, 0));
                    constexpr index_t c_offset_1 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 1, 0));
                    constexpr index_t c_offset_2 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 2, 0));
                    constexpr index_t c_offset_3 =
                        CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, 3, 0));

                    amd_assembly_outer_product_1x4(a_buf[Number<a_offset>{}],
                                                   b_buf[Number<b_offset_0>{}],
                                                   b_buf[Number<b_offset_1>{}],
                                                   b_buf[Number<b_offset_2>{}],
                                                   b_buf[Number<b_offset_3>{}],
                                                   c_buf(Number<c_offset_0>{}),
                                                   c_buf(Number<c_offset_1>{}),
                                                   c_buf(Number<c_offset_2>{}),
                                                   c_buf(Number<c_offset_3>{}));
                }
                else
                {
                    static_for<0, H, 1>{}([&](auto h) {
                        static_for<0, W, 1>{}([&](auto w) {

                            constexpr index_t b_offset =
                                BDesc{}.CalculateOffset(b_origin_idx + make_tuple(e, 0, h, w));

                            constexpr index_t c_offset =
                                CDesc{}.CalculateOffset(c_origin_idx + make_tuple(k, 0, h, w));

#if 0
                            c_buf(Number<c_offset>{}) += inner_product_with_conversion<FloatC>{}(a_buf[Number<a_offset>{}],
                                                                                               b_buf[Number<b_offset>{}]);
#else
                            amd_assembly_inner_product(a_buf[Number<a_offset>{}],
                                                       b_buf[Number<b_offset>{}],
                                                       c_buf(Number<c_offset>{}));
#endif
                        });
                    });
                }
            });
        });
    }
};
#endif

} // namespace ck
#endif
