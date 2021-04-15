#ifndef CK_GRIDWISE_DYNAMIC_GEMM_V2_HPP
#define CK_GRIDWISE_DYNAMIC_GEMM_V2_HPP

#include "common_header.hpp"
#include "dynamic_multi_index_transform_helper.hpp"
#include "dynamic_tensor_descriptor.hpp"
#include "dynamic_tensor_descriptor_helper.hpp"
#include "blockwise_dynamic_tensor_slice_transfer.hpp"
#include "threadwise_dynamic_tensor_slice_transfer.hpp"
#include "blockwise_gemm_v3.hpp"

namespace ck {

template <index_t BlockSize,
          typename FloatAB,
          typename FloatAcc,
          typename FloatC,
          InMemoryDataOperation CGlobalMemoryDataOperation,
          typename AGlobalDesc,
          typename BGlobalDesc,
          typename CGlobalDesc,
          index_t KPerBlock,
          index_t HoPerBlock,
          index_t WoPerBlock,
          index_t EPerBlock,
          index_t KPerThread,
          index_t HoPerThread,
          index_t WoPerThread,
          index_t EPerThread,
          typename ABlockTransferThreadSliceLengths_E_K,
          typename ABlockTransferThreadClusterLengths_E_K,
          typename ABlockTransferThreadClusterArrangeOrder,
          typename ABlockTransferSrcAccessOrder,
          index_t ABlockTransferSrcVectorDim,
          index_t ABlockTransferSrcScalarPerVector,
          index_t ABlockTransferDstScalarPerVector_K,
          bool AThreadTransferSrcResetCoordinateAfterRun,
          typename BBlockTransferSrcAccessOrder,
          index_t BBlockTransferSrcVectorDim,
          index_t BBlockTransferSrcScalarPerVector,
          bool BThreadTransferSrcResetCoordinateAfterRun,
          typename CThreadTransferSrcDstAccessOrder,
          index_t CThreadTransferSrcDstVectorDim,
          index_t CThreadTransferDstScalarPerVector,
          typename AGlobalIteratorHacks,
          typename BGlobalIteratorHacks,
          typename CGlobalIteratorHacks,
          typename AGlobalMoveSliceWindowIteratorHacks,
          typename BGlobalMoveSliceWindowIteratorHacks>
struct GridwiseDynamicGemm_km_kn_mn_v2
{
    __host__ __device__ static constexpr index_t GetSharedMemoryNumberOfByte()
    {
        constexpr auto E = EPerBlock * 3 * 3;

        constexpr auto max_lds_align =
            math::lcm(Number<ABlockTransferDstScalarPerVector_K>{}, Number<KPerBlock>{});

        // A matrix in LDS memory, dst of blockwise copy
        //   be careful of LDS alignment
        constexpr auto a_e_k_desc = make_dynamic_naive_tensor_descriptor_aligned_v2(
            make_tuple(Number<E>{}, Number<KPerBlock>{}), max_lds_align);

        // LDS allocation for A and B: be careful of alignment
        constexpr auto a_block_space_size =
            math::integer_least_multiple(a_e_k_desc.GetElementSpaceSize(), max_lds_align);

        return a_block_space_size * sizeof(FloatAB);
    }

    template <bool HasMainKBlockLoop, bool HasDoubleTailKBlockLoop>
    __device__ void Run(const AGlobalDesc& a_e_k_global_desc,
                        const FloatAB* __restrict__ p_a_global,
                        const BGlobalDesc& b_e_n_ho_wo_global_desc,
                        const FloatAB* __restrict__ p_b_global,
                        const CGlobalDesc& c_k_n_ho_wo_global_desc,
                        FloatC* __restrict__ p_c_global,
                        FloatAB* __restrict__ p_shared_block,
                        integral_constant<bool, HasMainKBlockLoop>,
                        integral_constant<bool, HasDoubleTailKBlockLoop>) const
    {
        constexpr auto I0 = Number<0>{};
        constexpr auto I1 = Number<1>{};
        constexpr auto I2 = Number<2>{};
        constexpr auto I3 = Number<3>{};

        constexpr auto E = EPerBlock * 3 * 3;

        const auto K = a_e_k_global_desc.GetLength(I1);

        const auto N  = b_e_n_ho_wo_global_desc.GetLength(I1);
        const auto Ho = b_e_n_ho_wo_global_desc.GetLength(I2);
        const auto Wo = b_e_n_ho_wo_global_desc.GetLength(I3);

        // divide block work by [M, N]
#if 0
        const auto k_block_work_num   = K / Number<KPerBlock>{};
        const auto ho_block_work_num  = Ho / Number<HoPerBlock>{};
        const auto wo_block_work_num  = Wo / Number<WoPerBlock>{};
        const auto hwo_block_work_num = ho_block_work_num * wo_block_work_num;

        const index_t k_block_work_id   = get_block_1d_id() / hwo_block_work_num;
        const index_t hwo_block_work_id = get_block_1d_id() - k_block_work_id * hwo_block_work_num;

        const index_t ho_block_work_id = hwo_block_work_id / wo_block_work_num;
        const index_t wo_block_work_id = hwo_block_work_id - ho_block_work_id * wo_block_work_num;
#else
        // Hack: this force result into SGPR
        const index_t k_block_work_num   = __builtin_amdgcn_readfirstlane(K / KPerBlock);
        const index_t ho_block_work_num  = __builtin_amdgcn_readfirstlane(Ho / HoPerBlock);
        const index_t wo_block_work_num  = __builtin_amdgcn_readfirstlane(Wo / WoPerBlock);
        const index_t hwo_block_work_num = ho_block_work_num * wo_block_work_num;

        const index_t k_block_work_id =
            __builtin_amdgcn_readfirstlane(get_block_1d_id() / hwo_block_work_num);
        const index_t hwo_block_work_id = get_block_1d_id() - k_block_work_id * hwo_block_work_num;

        const index_t ho_block_work_id =
            __builtin_amdgcn_readfirstlane(hwo_block_work_id / wo_block_work_num);
        const index_t wo_block_work_id = hwo_block_work_id - ho_block_work_id * wo_block_work_num;
#endif

        // lds max alignment
        constexpr auto max_lds_align =
            math::lcm(Number<ABlockTransferDstScalarPerVector_K>{}, Number<KPerBlock>{});

        // A matrix in LDS memory, dst of blockwise copy
        //   be careful of LDS alignment
        constexpr auto a_e_k_block_desc = make_dynamic_naive_tensor_descriptor_aligned_v2(
            make_tuple(Number<EPerBlock>{}, Number<KPerBlock>{}), max_lds_align);

        constexpr auto a_e_k_desc = make_dynamic_naive_tensor_descriptor_aligned_v2(
            make_tuple(Number<E>{}, Number<KPerBlock>{}), max_lds_align);

        // B matrix in LDS memory, dst of blockwise copy
        //   be careful of LDS alignment
        constexpr auto b_e_n_ho_wo_block_desc =
            make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(
                Number<EPerBlock>{}, Number<1>{}, Number<HoPerBlock>{}, Number<WoPerBlock>{}));

        // c_thread_mtx definition: this is a mess
        // TODO:: more elegent way of defining c_thread_mtx
        constexpr auto c_k_n_ho_wo_thread_desc =
            make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(
                Number<KPerThread>{}, Number<1>{}, Number<HoPerThread>{}, Number<WoPerThread>{}));

        const auto blockwise_gemm =
            BlockwiseGemm_km_kn_m0m1n0n1_v3<BlockSize,
                                            decltype(a_e_k_block_desc),
                                            decltype(b_e_n_ho_wo_block_desc),
                                            decltype(c_k_n_ho_wo_thread_desc),
                                            KPerThread,
                                            HoPerThread,
                                            WoPerThread,
                                            EPerThread,
                                            ABlockTransferSrcScalarPerVector,
                                            ABlockTransferDstScalarPerVector_K>{};

        // register allocation for output
        FloatAcc p_c_thread[c_k_n_ho_wo_thread_desc.GetElementSpaceSize()];

        // zero out threadwise output
        threadwise_matrix_set_zero_v3(c_k_n_ho_wo_thread_desc, p_c_thread);

        auto c_thread_mtx_index = blockwise_gemm.GetBeginOfThreadMatrixC(get_thread_local_1d_id());

        const auto k_thread_id  = c_thread_mtx_index.k;
        const auto ho_thread_id = c_thread_mtx_index.h;
        const auto wo_thread_id = c_thread_mtx_index.w;

        const index_t k_block_data_on_global  = k_block_work_id * KPerBlock;
        const index_t ho_block_data_on_global = ho_block_work_id * HoPerBlock;
        const index_t wo_block_data_on_global = wo_block_work_id * WoPerBlock;

        const index_t ho_thread_data_on_global =
            ho_block_data_on_global + ho_thread_id * HoPerThread;
        const index_t wo_thread_data_on_global =
            wo_block_data_on_global + wo_thread_id * WoPerThread;

#if 1
        // A matrix blockwise copy
        auto a_blockwise_copy =
            BlockwiseDynamicTensorSliceTransfer_v4<BlockSize,
                                                   InMemoryDataOperation::Set,
                                                   Sequence<E, KPerBlock>,
                                                   ABlockTransferThreadSliceLengths_E_K,
                                                   ABlockTransferThreadClusterLengths_E_K,
                                                   ABlockTransferThreadClusterArrangeOrder,
                                                   FloatAB,
                                                   FloatAB,
                                                   decltype(a_e_k_global_desc),
                                                   decltype(a_e_k_desc),
                                                   ABlockTransferSrcAccessOrder,
                                                   Sequence<0, 1>,
                                                   ABlockTransferSrcVectorDim,
                                                   1,
                                                   ABlockTransferSrcScalarPerVector,
                                                   ABlockTransferDstScalarPerVector_K,
                                                   AddressSpace::Global,
                                                   AddressSpace::Lds,
                                                   1,
                                                   1,
                                                   AThreadTransferSrcResetCoordinateAfterRun,
                                                   true>(
                a_e_k_global_desc,
                make_multi_index(0, k_block_data_on_global),
                a_e_k_desc,
                make_multi_index(0, 0));

        constexpr auto b_e_n_ho_wo_thread_desc =
            make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(
                Number<EPerBlock>{}, Number<1>{}, Number<HoPerThread>{}, Number<WoPerThread>{}));

        auto b_threadwise_transfer = ThreadwiseDynamicTensorSliceTransfer_v2<
            FloatAB,
            FloatAB,
            decltype(b_e_n_ho_wo_global_desc),
            decltype(b_e_n_ho_wo_thread_desc),
            Sequence<EPerBlock, 1, HoPerThread, WoPerThread>,
            BBlockTransferSrcAccessOrder,
            BBlockTransferSrcVectorDim,
            BBlockTransferSrcScalarPerVector,
            AddressSpace::Global,
            AddressSpace::Vgpr,
            InMemoryDataOperation::Set,
            1,
            true>(b_e_n_ho_wo_global_desc,
                  make_multi_index(0, 0, ho_thread_data_on_global, wo_thread_data_on_global));

        FloatAB* p_a_block = p_shared_block;

        constexpr auto b_thread_slice_copy_step = make_multi_index(EPerBlock, 0, 0, 0);

        // hack to control index calculation when iterating over A and B matrix for threadwise copy
        constexpr auto a_e_k_global_iterator_hacks       = AGlobalIteratorHacks{};
        constexpr auto b_e_n_ho_wo_global_iterator_hacks = BGlobalIteratorHacks{};

        // hack to control index calculation when move slice window for A and B matrix for
        // threadwise copy
        constexpr auto a_e_k_global_move_slice_window_iterator_hack =
            AGlobalMoveSliceWindowIteratorHacks{};
        constexpr auto b_e_n_ho_wo_global_move_slice_window_iterator_hack =
            BGlobalMoveSliceWindowIteratorHacks{};

        constexpr auto b_thread_space_size = b_e_n_ho_wo_thread_desc.GetElementSpaceSize();
        FloatAB p_b_thread_double[b_thread_space_size * 2];

        // LDS double buffer: preload data into LDS
        {
            a_blockwise_copy.RunRead(a_e_k_global_desc, p_a_global, a_e_k_global_iterator_hacks);

            b_threadwise_transfer.Run(b_e_n_ho_wo_global_desc,
                                      p_b_global,
                                      b_e_n_ho_wo_thread_desc,
                                      make_tuple(I0, I0, I0, I0),
                                      p_b_thread_double,
                                      b_e_n_ho_wo_global_iterator_hacks);

            a_blockwise_copy.RunWrite(a_e_k_desc, p_a_block);
        }

        __syncthreads();

        index_t b_block_data_begin = 0;

        if constexpr(HasMainKBlockLoop)
        {
            FloatAB* p_b_thread_even = p_b_thread_double;
            FloatAB* p_b_thread_odd  = p_b_thread_double + b_thread_space_size;

            // LDS double buffer: main body
            // use Do-While loop instead of For loop to simplify control flow
            do
            {
                // even iteration
                b_threadwise_transfer.MoveSrcSliceWindow(b_e_n_ho_wo_global_desc,
                                                         b_thread_slice_copy_step);

                b_threadwise_transfer.Run(b_e_n_ho_wo_global_desc,
                                          p_b_global,
                                          b_e_n_ho_wo_thread_desc,
                                          make_tuple(I0, I0, I0, I0),
                                          p_b_thread_odd,
                                          b_e_n_ho_wo_global_iterator_hacks);

                // LDS double buffer: GEMM on current data
                blockwise_gemm.Run(
                    p_a_block + a_e_k_block_desc.CalculateOffset(make_tuple(b_block_data_begin, 0)),
                    p_b_thread_even,
                    p_c_thread);

                b_block_data_begin += EPerBlock;

                b_threadwise_transfer.MoveSrcSliceWindow(b_e_n_ho_wo_global_desc,
                                                         b_thread_slice_copy_step);

                b_threadwise_transfer.Run(b_e_n_ho_wo_global_desc,
                                          p_b_global,
                                          b_e_n_ho_wo_thread_desc,
                                          make_tuple(I0, I0, I0, I0),
                                          p_b_thread_even,
                                          b_e_n_ho_wo_global_iterator_hacks);

                // LDS double buffer: GEMM on current data
                blockwise_gemm.Run(
                    p_a_block + a_e_k_block_desc.CalculateOffset(make_tuple(b_block_data_begin, 0)),
                    p_b_thread_odd,
                    p_c_thread);

                b_block_data_begin += EPerBlock;

            } while(b_block_data_begin < E - 2 * EPerBlock);
        }

        // LDS double buffer: tail
        if constexpr(HasDoubleTailKBlockLoop) // if has 2 iteration left
        {
            b_threadwise_transfer.MoveSrcSliceWindow(b_e_n_ho_wo_global_desc,
                                                     b_thread_slice_copy_step);

            b_threadwise_transfer.Run(b_e_n_ho_wo_global_desc,
                                      p_b_global,
                                      b_e_n_ho_wo_thread_desc,
                                      make_tuple(I0, I0, I0, I0),
                                      p_b_thread_double + b_thread_space_size,
                                      b_e_n_ho_wo_global_iterator_hacks);

            // LDS double buffer: GEMM on 2nd-last data
            blockwise_gemm.Run(
                p_a_block + a_e_k_block_desc.CalculateOffset(make_tuple(b_block_data_begin, 0)),
                p_b_thread_double,
                p_c_thread);

            b_block_data_begin += EPerBlock;

            // LDS double buffer: GEMM on last data
            blockwise_gemm.Run(
                p_a_block + a_e_k_block_desc.CalculateOffset(make_tuple(b_block_data_begin, 0)),
                p_b_thread_double + b_thread_space_size,
                p_c_thread);
        }
        else // if has 1 iteration left
        {
            // LDS double buffer: GEMM on last data
            blockwise_gemm.Run(
                p_a_block + a_e_k_block_desc.CalculateOffset(make_tuple(b_block_data_begin, 0)),
                p_b_thread_double,
                p_c_thread);
        }
#endif

        // output: register to global memory
        {

            static_assert(CThreadTransferDstScalarPerVector == 16 && KPerBlock == 16, "");
            const index_t k_block_data_on_global_vec =
                k_block_work_id * (KPerBlock / CThreadTransferDstScalarPerVector);
            const index_t KPerThreadVec = KPerThread / CThreadTransferDstScalarPerVector;
            const index_t k_thread_data_on_global_vec =
                k_block_data_on_global_vec + k_thread_id * KPerThreadVec;

            constexpr auto c_k_n_ho_wo_thread_desc_vec =
                make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(Number<KPerThreadVec>{},
                                                                          Number<1>{},
                                                                          Number<HoPerThread>{},
                                                                          Number<WoPerThread>{}));

            static_assert(c_k_n_ho_wo_thread_desc_vec.GetElementSpaceSize() == 4, "");

            const index_t vec_len = c_k_n_ho_wo_thread_desc_vec.GetElementSpaceSize() *
                                    CThreadTransferDstScalarPerVector;

            vector_type<int8_t, vec_len> d_vec;

            // FloatC d_vec[c_k_n_ho_wo_thread_desc_vec.GetElementSpaceSize()];

            constexpr auto c_k_n_ho_wo_global_tensor_iterator_hacks = CGlobalIteratorHacks{};

            static_for<0, KPerThreadVec, 1>{}([&](auto k_i) {
                static_for<0, HoPerThread, 1>{}([&](auto h_i) {
                    static_for<0, WoPerThread, 1>{}([&](auto w_i) {
                        vector_type<int8_t, CThreadTransferDstScalarPerVector> t;

                        t.template AsType<FloatC>()(Number<0>{}) = d_vec.template AsType<
                            FloatC>()[Number<c_k_n_ho_wo_thread_desc_vec.CalculateOffset(
                            make_tuple(k_i, 0, h_i, w_i))>{}];

                        // t.template AsType<FloatC>()(Number<0>{}) =
                        // d_vec[Number<c_k_n_ho_wo_thread_desc_vec.CalculateOffset(
                        // make_tuple(k_i, 0, h_i, w_i))>{}];

                        static_for<0, CThreadTransferDstScalarPerVector, 1>{}([&](auto i) {
                            t.template AsType<int8_t>()(i) =
                                p_c_thread[c_k_n_ho_wo_thread_desc_vec.CalculateOffset(make_tuple(
                                    k_i * CThreadTransferDstScalarPerVector + i, 0, h_i, w_i))];
                        });

                        d_vec.template AsType<FloatC>()(
                            Number<c_k_n_ho_wo_thread_desc_vec.CalculateOffset(make_tuple(
                                k_i, 0, h_i, w_i))>{}) = t.template AsType<FloatC>()[Number<0>{}];

                        // d_vec[Number<c_k_n_ho_wo_thread_desc_vec.CalculateOffset(make_tuple(
                        // k_i, 0, h_i, w_i))>{}] = t.template AsType<FloatC>()[Number<0>{}];
                    });
                });
            });

            ThreadwiseDynamicTensorSliceTransfer_v1r3<
                // FloatC,
                decltype(d_vec),
                FloatC,
                decltype(c_k_n_ho_wo_thread_desc_vec),
                decltype(c_k_n_ho_wo_global_desc),
                Sequence<KPerThreadVec, 1, HoPerThread, WoPerThread>,
                CThreadTransferSrcDstAccessOrder,
                CThreadTransferSrcDstVectorDim,
                // CThreadTransferDstScalarPerVector,
                1,
                AddressSpace::Vgpr,
                AddressSpace::Global,
                CGlobalMemoryDataOperation,
                1,
                true>(c_k_n_ho_wo_global_desc,
                      make_multi_index(k_thread_data_on_global_vec,
                                       0,
                                       ho_thread_data_on_global,
                                       wo_thread_data_on_global))
                .Run(c_k_n_ho_wo_thread_desc_vec,
                     make_tuple(I0, I0, I0, I0),
                     d_vec,
                     c_k_n_ho_wo_global_desc,
                     p_c_global,
                     c_k_n_ho_wo_global_tensor_iterator_hacks);
        }
    }

    // pass tensor descriptor by reference
    template <bool HasMainKBlockLoop, bool HasDoubleTailKBlockLoop>
    __device__ void Run(const AGlobalDesc& a_e_k_global_desc,
                        const FloatAB* __restrict__ p_a_global,
                        const BGlobalDesc& b_e_n_ho_wo_global_desc,
                        const FloatAB* __restrict__ p_b_global,
                        const CGlobalDesc& c_k_n_ho_wo_global_desc,
                        FloatC* __restrict__ p_c_global,
                        integral_constant<bool, HasMainKBlockLoop>,
                        integral_constant<bool, HasDoubleTailKBlockLoop>) const
    {
        constexpr index_t shared_block_size = GetSharedMemoryNumberOfByte() / sizeof(FloatAB);

        __shared__ FloatAB p_shared_block[shared_block_size];

        Run(a_e_k_global_desc,
            p_a_global,
            b_e_n_ho_wo_global_desc,
            p_b_global,
            c_k_n_ho_wo_global_desc,
            p_c_global,
            p_shared_block,
            integral_constant<bool, HasMainKBlockLoop>{},
            integral_constant<bool, HasDoubleTailKBlockLoop>{});
    }

    // pass tensor descriptors by their pointers
    template <bool HasMainKBlockLoop, bool HasDoubleTailKBlockLoop>
    __device__ void Run(const AGlobalDesc* p_a_e_k_global_desc,
                        const FloatAB* __restrict__ p_a_global,
                        const BGlobalDesc* p_b_e_n_ho_wo_global_desc,
                        const FloatAB* __restrict__ p_b_global,
                        const CGlobalDesc* p_c_k_n_ho_wo_global_desc,
                        FloatC* __restrict__ p_c_global,
                        integral_constant<bool, HasMainKBlockLoop>,
                        integral_constant<bool, HasDoubleTailKBlockLoop>) const
    {
        const auto a_e_k_global_desc       = *p_a_e_k_global_desc;
        const auto b_e_n_ho_wo_global_desc = *p_b_e_n_ho_wo_global_desc;
        const auto c_k_n_ho_wo_global_desc = *p_c_k_n_ho_wo_global_desc;

        Run(a_e_k_global_desc,
            p_a_global,
            b_e_n_ho_wo_global_desc,
            p_b_global,
            c_k_n_ho_wo_global_desc,
            p_c_global,
            integral_constant<bool, HasMainKBlockLoop>{},
            integral_constant<bool, HasDoubleTailKBlockLoop>{});
    }

    // pass tensor descriptors by void*
    template <bool HasMainKBlockLoop, bool HasDoubleTailKBlockLoop>
    __device__ void Run(const void* p_a_e_k_global_desc,
                        const FloatAB* __restrict__ p_a_global,
                        const void* p_b_e_n_ho_wo_global_desc,
                        const FloatAB* __restrict__ p_b_global,
                        const void* p_c_k_n_ho_wo_global_desc,
                        FloatC* __restrict__ p_c_global,
                        integral_constant<bool, HasMainKBlockLoop>,
                        integral_constant<bool, HasDoubleTailKBlockLoop>) const
    {
        const auto a_e_k_global_desc = *reinterpret_cast<const AGlobalDesc*>(p_a_e_k_global_desc);
        const auto b_e_n_ho_wo_global_desc =
            *reinterpret_cast<const BGlobalDesc*>(p_b_e_n_ho_wo_global_desc);
        const auto c_k_n_ho_wo_global_desc =
            *reinterpret_cast<const CGlobalDesc*>(p_c_k_n_ho_wo_global_desc);

        Run(a_e_k_global_desc,
            p_a_global,
            b_e_n_ho_wo_global_desc,
            p_b_global,
            c_k_n_ho_wo_global_desc,
            p_c_global,
            integral_constant<bool, HasMainKBlockLoop>{},
            integral_constant<bool, HasDoubleTailKBlockLoop>{});
    }
}; // namespace ck

} // namespace ck
#endif
