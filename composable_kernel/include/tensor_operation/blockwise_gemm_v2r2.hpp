#ifndef CK_BLOCKWISE_GEMM_V2R2_HPP
#define CK_BLOCKWISE_GEMM_V2R2_HPP

#include "common_header.hpp"
#include "threadwise_dynamic_tensor_slice_transfer.hpp"
#include "threadwise_gemm_v2.hpp"

namespace ck {

// C[M0, M1, N0, N1] += transpose(A[K, M0, M1]) * B[K, N0, N1]
// A and B are visable to the whole block, C is distributed among each thread
// Assume:
//   1. A:
//     1. AKMBlockDesc is known at compile-time
//     2. ABlockBuffer is DynamicBuffer
//   2. B:
//     1. AKMBlockDesc is known at compile-time
//     2. BBlockBuffer is DynamicBuffer
//   3. C:
//     1. CM0M1N0N1ThreadDesc is known at compile-time
//     2. CThreadBuffer is StaticBuffer
// Also assume:
//   M0 = N0 = 2. It will do 2x2 pipelined read and fma (ABBA optimization)
template <index_t BlockSize,
          typename FloatA,
          typename FloatB,
          typename FloatC,
          typename AKMBlockDesc,
          typename BKNBlockDesc,
          typename CM0M1N0N1ThreadDesc,
          index_t M1PerThreadM11,
          index_t N1PerThreadN11,
          index_t KPerThread,
          index_t M1N1ThreadClusterM100,
          index_t M1N1ThreadClusterN100,
          index_t M1N1ThreadClusterM101,
          index_t M1N1ThreadClusterN101,
          index_t AThreadCopyScalarPerVector_M11,
          index_t BThreadCopyScalarPerVector_N11,
          typename std::enable_if<AKMBlockDesc::IsKnownAtCompileTime() &&
                                      BKNBlockDesc::IsKnownAtCompileTime() &&
                                      CM0M1N0N1ThreadDesc::IsKnownAtCompileTime(),
                                  bool>::type = false>
struct BlockwiseGemm_km0m1_kn0n1_m0m1n0n1_v2r2_pipeline_2x2
{
    using AIndex = MultiIndex<3>;
    using BIndex = MultiIndex<3>;
    using CIndex = MultiIndex<4>;

    static constexpr auto I0 = Number<0>{};
    static constexpr auto I1 = Number<1>{};
    static constexpr auto I2 = Number<2>{};
    static constexpr auto I3 = Number<3>{};

    public:
    __device__ BlockwiseGemm_km0m1_kn0n1_m0m1n0n1_v2r2_pipeline_2x2()
        : c_thread_origin_data_idx_{CalculateCThreadOriginDataIndex(get_thread_local_1d_id())},
          a_thread_copy_{
              make_tuple(0, c_thread_origin_data_idx_[I0], c_thread_origin_data_idx_[I1])},
          b_thread_copy_{
              make_tuple(0, c_thread_origin_data_idx_[I2], c_thread_origin_data_idx_[I3])}
    {
        static_assert(AKMBlockDesc::IsKnownAtCompileTime() &&
                          BKNBlockDesc::IsKnownAtCompileTime() &&
                          CM0M1N0N1ThreadDesc::IsKnownAtCompileTime(),
                      "wrong! Desc should be known at compile-time");

        static_assert(BlockSize == M1N1ThreadClusterM101 * M1N1ThreadClusterM100 *
                                       M1N1ThreadClusterN101 * M1N1ThreadClusterN100,
                      "wrong! blocksize and cluster size not consistent");

        static_assert(AKMBlockDesc{}.GetLength(I0) == BKNBlockDesc{}.GetLength(I0),
                      "wrong! K dimension not consistent");

        // TODO: remove this restriction
        static_assert(AKMBlockDesc{}.GetLength(I1) == 2 && BKNBlockDesc{}.GetLength(I1) == 2 &&
                          CM0M1N0N1ThreadDesc{}.GetLength(I0) == 2 &&
                          CM0M1N0N1ThreadDesc{}.GetLength(I2) == 2,
                      "wrong");
    }

    __device__ static CIndex CalculateCThreadOriginDataIndex(index_t thread_id)
    {
        constexpr index_t M0 = AKMBlockDesc{}.GetLength(I1);
        constexpr index_t N0 = BKNBlockDesc{}.GetLength(I1);
        constexpr index_t M1 = AKMBlockDesc{}.GetLength(I2);
        constexpr index_t N1 = BKNBlockDesc{}.GetLength(I2);

        // 4-d data space into 4-d thread space
        constexpr auto adaptor0 = make_single_stage_tensor_adaptor(
            make_tuple(make_vectorize_transform(M0, 1),
                       make_vectorize_transform(M1PerThreadM11, M1 / M1PerThreadM11),
                       make_vectorize_transform(N0, 1),
                       make_vectorize_transform(N1PerThreadN11, N1 / N1PerThreadN11)),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}));

        // thread position 4-d thread space
        constexpr auto adaptor1 = make_single_stage_tensor_adaptor(
            make_tuple(
                make_freeze_transform(make_multi_index(0)),
                make_unmerge_transform(make_tuple(M1N1ThreadClusterM100, M1N1ThreadClusterM101)),
                make_freeze_transform(make_multi_index(0)),
                make_unmerge_transform(make_tuple(M1N1ThreadClusterN100, M1N1ThreadClusterN101))),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}),
            make_tuple(Sequence<>{}, Sequence<0, 1>{}, Sequence<>{}, Sequence<2, 3>{}));

        // 4-d thread space to 1-d thread space
        constexpr auto adaptor2 = make_single_stage_tensor_adaptor(
            make_tuple(make_merge_transform(make_tuple(M1N1ThreadClusterM100,
                                                       M1N1ThreadClusterN100,
                                                       M1N1ThreadClusterM101,
                                                       M1N1ThreadClusterN101))),
            make_tuple(Sequence<0, 2, 1, 3>{}),
            make_tuple(Sequence<0>{}));

        constexpr auto cluster_desc = chain_tensor_adaptors(adaptor0, adaptor1, adaptor2);

        return cluster_desc.CalculateBottomIndex(make_multi_index(get_thread_local_1d_id()));
    }

    template <typename ABlockBuffer, typename BBlockBuffer, typename CThreadBuffer>
    __device__ void Run(const ABlockBuffer& a_block_buf,
                        const BBlockBuffer& b_block_buf,
                        CThreadBuffer& c_thread_buf) const
    {
        auto a_thread_buf =
            make_static_buffer<AddressSpace::Vgpr, FloatA>(a_thread_desc_.GetElementSpaceSize());
        auto b_thread_buf =
            make_static_buffer<AddressSpace::Vgpr, FloatB>(b_thread_desc_.GetElementSpaceSize());

        constexpr auto threadwise_gemm =
            ThreadwiseGemm_km0m1_kn0n1_m0m1n0n1<FloatA,
                                                FloatB,
                                                FloatC,
                                                decltype(a_thread_desc_),
                                                decltype(b_thread_desc_),
                                                CM0M1N0N1ThreadDesc,
                                                Sequence<KPerThread>,
                                                Sequence<1, M1PerThreadM11>,
                                                Sequence<1, N1PerThreadN11>>{};

        constexpr index_t K = AKMBlockDesc{}.GetLength(I0);

        // read A_sub_0
        a_thread_copy_.Run(AKMBlockDesc{},
                           make_tuple(I0, I0, I0),
                           a_block_buf,
                           a_thread_desc_,
                           make_tuple(I0, I0, I0),
                           a_thread_buf);

        // read B_sub_0
        b_thread_copy_.Run(BKNBlockDesc{},
                           make_tuple(I0, I0, I0),
                           b_block_buf,
                           b_thread_desc_,
                           make_tuple(I0, I0, I0),
                           b_thread_buf);

        // read B_sub_1
        b_thread_copy_.Run(BKNBlockDesc{},
                           make_tuple(I0, I1, I0),
                           b_block_buf,
                           b_thread_desc_,
                           make_tuple(I0, I1, I0),
                           b_thread_buf);

        // read A_sub_1
        a_thread_copy_.Run(AKMBlockDesc{},
                           make_tuple(I0, I1, I0),
                           a_block_buf,
                           a_thread_desc_,
                           make_tuple(I0, I1, I0),
                           a_thread_buf);

        // C_sub_00 += transpose(A_sub_0) * B_sub_0
        threadwise_gemm.Run(a_thread_buf,
                            make_tuple(I0, I0, I0),
                            b_thread_buf,
                            make_tuple(I0, I0, I0),
                            c_thread_buf,
                            make_tuple(I0, I0, I0, I0));

        // C_sub_01 += transpose(A_sub_0) * B_sub_1
        threadwise_gemm.Run(a_thread_buf,
                            make_tuple(I0, I0, I0),
                            b_thread_buf,
                            make_tuple(I0, I1, I0),
                            c_thread_buf,
                            make_tuple(I0, I0, I1, I0));

        // loop over rest of k
        static_for<KPerThread, K, KPerThread>{}([&](auto k) {
            // read A_sub_0
            a_thread_copy_.Run(AKMBlockDesc{},
                               make_tuple(k, I0, I0),
                               a_block_buf,
                               a_thread_desc_,
                               make_tuple(I0, I0, I0),
                               a_thread_buf);

            // C_sub_10 += transpose(A_sub_1) * B_sub_0
            threadwise_gemm.Run(a_thread_buf,
                                make_tuple(I0, I1, I0),
                                b_thread_buf,
                                make_tuple(I0, I0, I0),
                                c_thread_buf,
                                make_tuple(I1, I0, I0, I0));

            // read B_sub_0
            b_thread_copy_.Run(BKNBlockDesc{},
                               make_tuple(k, I0, I0),
                               b_block_buf,
                               b_thread_desc_,
                               make_tuple(I0, I0, I0),
                               b_thread_buf);

            // C_sub_11 += transpose(A_sub_1) * B_sub_1
            threadwise_gemm.Run(a_thread_buf,
                                make_tuple(I0, I1, I0),
                                b_thread_buf,
                                make_tuple(I0, I1, I0),
                                c_thread_buf,
                                make_tuple(I1, I0, I1, I0));

            // read B_sub_1
            b_thread_copy_.Run(BKNBlockDesc{},
                               make_tuple(k, I1, I0),
                               b_block_buf,
                               b_thread_desc_,
                               make_tuple(I0, I1, I0),
                               b_thread_buf);

            // read A_sub_1
            a_thread_copy_.Run(AKMBlockDesc{},
                               make_tuple(k, I1, I0),
                               a_block_buf,
                               a_thread_desc_,
                               make_tuple(I0, I1, I0),
                               a_thread_buf);

            // C_sub_00 += transpose(A_sub_0) * B_sub_0
            threadwise_gemm.Run(a_thread_buf,
                                make_tuple(I0, I0, I0),
                                b_thread_buf,
                                make_tuple(I0, I0, I0),
                                c_thread_buf,
                                make_tuple(I0, I0, I0, I0));

            // C_sub_01 += transpose(A_sub_0) * B_sub_1
            threadwise_gemm.Run(a_thread_buf,
                                make_tuple(I0, I0, I0),
                                b_thread_buf,
                                make_tuple(I0, I1, I0),
                                c_thread_buf,
                                make_tuple(I0, I0, I1, I0));
        });

        // C_sub_10 += transpose(A_sub_1) * B_sub_0
        threadwise_gemm.Run(a_thread_buf,
                            make_tuple(I0, I1, I0),
                            b_thread_buf,
                            make_tuple(I0, I0, I0),
                            c_thread_buf,
                            make_tuple(I1, I0, I0, I0));

        // C_sub_11 += transpose(A_sub_1) * B_sub_1
        threadwise_gemm.Run(a_thread_buf,
                            make_tuple(I0, I1, I0),
                            b_thread_buf,
                            make_tuple(I0, I1, I0),
                            c_thread_buf,
                            make_tuple(I1, I0, I1, I0));
    }

    private:
    static constexpr index_t M0_ = AKMBlockDesc{}.GetLength(I1);
    static constexpr index_t N0_ = BKNBlockDesc{}.GetLength(I1);

    // A[K, M0, M1]
    static constexpr auto a_thread_desc_ = make_dynamic_naive_tensor_descriptor_packed_v2(
        make_tuple(Number<KPerThread>{}, Number<M0_>{}, Number<M1PerThreadM11>{}));

    // B[K, N0, N1]
    static constexpr auto b_thread_desc_ = make_dynamic_naive_tensor_descriptor_packed_v2(
        make_tuple(Number<KPerThread>{}, Number<N0_>{}, Number<N1PerThreadN11>{}));

    using AThreadCopy =
        ThreadwiseDynamicTensorSliceTransfer_v4<FloatA,
                                                FloatA,
                                                AKMBlockDesc,
                                                decltype(a_thread_desc_),
                                                Sequence<KPerThread, 1, M1PerThreadM11>,
                                                Sequence<0, 1, 2>,
                                                2,
                                                AThreadCopyScalarPerVector_M11,
                                                1>;

    using BThreadCopy =
        ThreadwiseDynamicTensorSliceTransfer_v4<FloatB,
                                                FloatB,
                                                BKNBlockDesc,
                                                decltype(b_thread_desc_),
                                                Sequence<KPerThread, 1, N1PerThreadN11>,
                                                Sequence<0, 1, 2>,
                                                2,
                                                BThreadCopyScalarPerVector_N11,
                                                1>;

    CIndex c_thread_origin_data_idx_;

    AThreadCopy a_thread_copy_;
    BThreadCopy b_thread_copy_;
};

} // namespace ck
#endif
