#pragma once

#include "cluster_descriptor.hpp"
#include "data_type.hpp"
#include "element_wise_operation.hpp"
#include "threadwise_tensor_slice_transfer.hpp"

namespace ck {

template <typename GridwiseEltwise,
          typename ADataType,
          typename BDataType,
          typename CDataType,
          typename GridDesc_M0,
          typename ElementwiseFunctor>
__global__ void kernel_elementwise_1d(const ADataType* __restrict__ p_a_global,
                                      const BDataType* __restrict__ p_b_global,
                                      CDataType* __restrict__ p_c_global,
                                      const GridDesc_M0 a_grid_desc_m0,
                                      const GridDesc_M0 b_grid_desc_m0,
                                      const GridDesc_M0 c_grid_desc_m0,
                                      const ElementwiseFunctor functor)
{
    GridwiseEltwise::Run(p_a_global,
                         p_b_global,
                         p_c_global,
                         a_grid_desc_m0,
                         b_grid_desc_m0,
                         c_grid_desc_m0,
                         functor);
}

template <typename ADataType,
          typename BDataType,
          typename CDataType,
          typename GridDesc_M0,
          typename ElementwiseFunctor,
          index_t ThreadPerBlock,
          index_t ThreadTileSize,
          index_t ScalarPerVector>
struct GridwiseElementwise_1D
{
    static constexpr auto I0           = Number<0>{};
    static constexpr int BlockTileSize = ThreadPerBlock * ThreadTileSize;
    static constexpr auto thread_desc_M0 =
        make_naive_tensor_descriptor_packed(make_tuple(Number<ScalarPerVector>{}));

    using PassThrough = tensor_operation::element_wise::PassThrough;

    static __device__ __host__ auto CalculateElementwiseIndex()
    {
        const index_t thread_id = get_thread_local_1d_id();
        const index_t block_id  = get_block_1d_id();

        return make_multi_index(block_id * BlockTileSize + thread_id * ScalarPerVector);
    }

    __device__ static void Run(const ADataType* __restrict__ p_a_global,
                               const BDataType* __restrict__ p_b_global,
                               CDataType* __restrict__ p_c_global,
                               const GridDesc_M0 a_grid_desc_m0,
                               const GridDesc_M0 b_grid_desc_m0,
                               const GridDesc_M0 c_grid_desc_m0,
                               const ElementwiseFunctor functor)
    {
        const auto a_global_buf = make_dynamic_buffer<AddressSpaceEnum::Global>(
            p_a_global, a_grid_desc_m0.GetElementSpaceSize());
        const auto b_global_buf = make_dynamic_buffer<AddressSpaceEnum::Global>(
            p_b_global, b_grid_desc_m0.GetElementSpaceSize());
        auto c_global_buf = make_dynamic_buffer<AddressSpaceEnum::Global>(
            p_c_global, c_grid_desc_m0.GetElementSpaceSize());

        StaticBuffer<AddressSpaceEnum::Vgpr, ADataType, ScalarPerVector, true> a_thread_buf;
        StaticBuffer<AddressSpaceEnum::Vgpr, BDataType, ScalarPerVector, true> b_thread_buf;
        StaticBuffer<AddressSpaceEnum::Vgpr, CDataType, ScalarPerVector, true> c_thread_buf;

        const auto thread_to_global_offset = CalculateElementwiseIndex();

        auto a_global_load =
            ThreadwiseTensorSliceTransfer_v2<ADataType,
                                             ADataType,
                                             GridDesc_M0,
                                             decltype(thread_desc_M0),
                                             Sequence<ScalarPerVector>, // SliceLengths
                                             Sequence<0>,               // DimAccessOrder
                                             0,                         // SrcVectorDim
                                             ScalarPerVector,
                                             1, // SrcScalarStrideInVector
                                             false>{a_grid_desc_m0, thread_to_global_offset};

        auto b_global_load =
            ThreadwiseTensorSliceTransfer_v2<BDataType,
                                             BDataType,
                                             GridDesc_M0,
                                             decltype(thread_desc_M0),
                                             Sequence<ScalarPerVector>, // SliceLengths
                                             Sequence<0>,               // DimAccessOrder
                                             0,                         // SrcVectorDim
                                             ScalarPerVector,
                                             1, // SrcScalarStrideInVector
                                             false>{b_grid_desc_m0, thread_to_global_offset};

        auto c_global_write =
            ThreadwiseTensorSliceTransfer_v1r3<CDataType,
                                               CDataType,
                                               decltype(thread_desc_M0),
                                               GridDesc_M0,
                                               PassThrough,
                                               Sequence<ScalarPerVector>, // SliceLengths
                                               Sequence<0>,               // DimAccessOrder
                                               0,                         // DstVectorDim
                                               ScalarPerVector,
                                               InMemoryDataOperationEnum::Set,
                                               1, // DstScalarStrideInVector
                                               false>{
                c_grid_desc_m0, thread_to_global_offset, PassThrough{}};

        int num_iter                         = ThreadTileSize / ScalarPerVector;
        constexpr auto thread_to_global_step = make_multi_index(ThreadPerBlock * ScalarPerVector);
        do
        {
            // read and process ScalarPerVector elements
            a_global_load.Run(
                a_grid_desc_m0, a_global_buf, thread_desc_M0, make_tuple(I0), a_thread_buf);

            b_global_load.Run(
                b_grid_desc_m0, b_global_buf, thread_desc_M0, make_tuple(I0), b_thread_buf);

            static_for<0, ScalarPerVector, 1>{}([&](auto m) {
                constexpr auto offset = thread_desc_M0.CalculateOffset(make_tuple(m));
                functor(c_thread_buf(Number<offset>{}),
                        a_thread_buf(Number<offset>{}),
                        b_thread_buf(Number<offset>{}));
            });

            c_global_write.Run(thread_desc_M0,
                               make_tuple(I0), // SrcSliceOriginIdx
                               c_thread_buf,
                               c_grid_desc_m0,
                               c_global_buf);

            a_global_load.MoveSrcSliceWindow(a_grid_desc_m0, thread_to_global_step);
            b_global_load.MoveSrcSliceWindow(b_grid_desc_m0, thread_to_global_step);
            c_global_write.MoveDstSliceWindow(c_grid_desc_m0, thread_to_global_step);
        } while(--num_iter);
    }
};

} // namespace ck
