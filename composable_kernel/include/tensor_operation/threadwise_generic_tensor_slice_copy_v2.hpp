#ifndef CK_THREADWISE_GENERIC_TENSOR_SLICE_COPY_V2_HPP
#define CK_THREADWISE_GENERIC_TENSOR_SLICE_COPY_V2_HPP

#include "common_header.hpp"
#include "tensor_descriptor.hpp"
#include "tensor_descriptor_helper.hpp"
#include "tensor_coordinate.hpp"

namespace ck {

// This threadwise copy allow vector access of src and dst.
// It allows the vector size to be different on src and dst.
// The dimensions of vector access should be the same on src and dst.
// The dimension access order should be the same on src and dst.
// Will do valid mapping check on src data: Read 0 if src data has a invalid mapping
// Will do valid mapping check on dst data: No write if dst data has a invalid mapping
template <typename SrcDesc,
          typename DstDesc,
          typename SliceLengths,
          typename SrcDimAccessOrder,
          typename DstDimAccessOrder,
          index_t SrcVectorReadDim,
          index_t DstVectorWriteDim,
          index_t SrcDataPerRead,
          index_t DstDataPerWrite,
          AddressSpace SrcAddressSpace     = AddressSpace::Generic,
          AddressSpace DstAddressSpace     = AddressSpace::Generic,
          InMemoryDataOperation DstInMemOp = InMemoryDataOperation::Set,
          index_t SrcDataStride            = 1,
          index_t DstDataStride            = 1>
struct ThreadwiseGenericTensorSliceCopy_v5
{
    using ThreadBufferDesc = decltype(make_native_tensor_descriptor_packed(SliceLengths{}));

    static constexpr index_t nDim = SliceLengths::Size();
    using Index                   = MultiIndex<nDim>;

    using SrcCoord = typename TensorCoordinate<SrcDesc>::type;
    using DstCoord = typename TensorCoordinate<DstDesc>::type;

    __device__ constexpr ThreadwiseGenericTensorSliceCopy_v5(const Index& src_slice_origin,
                                                             const Index& dst_slice_origin)
        : mSrcSliceOrigin(src_slice_origin), mDstSliceOrigin(dst_slice_origin)
    {
        static_assert(nDim == SrcDesc::GetNumOfDimension() &&
                          nDim == DstDesc::GetNumOfDimension() && nDim == SliceLengths::Size() &&
                          nDim == SrcDimAccessOrder::Size() && nDim == DstDimAccessOrder::Size(),
                      "wrong! # of dimensions not the same");

        static_assert(is_valid_sequence_map<SrcDimAccessOrder>{}, "wrong! map is not valid");
        static_assert(is_valid_sequence_map<DstDimAccessOrder>{}, "wrong! map is not valid");

        static_assert(SliceLengths{}[SrcVectorReadDim] % SrcDataPerRead == 0,
                      "wrong! cannot evenly divide");

        static_assert(SliceLengths{}[DstVectorWriteDim] % DstDataPerWrite == 0,
                      "wrong! cannot evenly divide");
    }

    __device__ constexpr ThreadwiseGenericTensorSliceCopy_v5()
        : ThreadwiseGenericTensorSliceCopy_v5(make_zero_multi_index<nDim>(),
                                              make_zero_multi_index<nDim>())
    {
    }

    __device__ void SetSrcSliceOrigin(SrcCoord src_slice_origin)
    {
        mSrcSliceOrigin = src_slice_origin;
    }

    __device__ void SetDstSliceOrigin(DstCoord dst_slice_origin)
    {
        mDstSliceOrigin = dst_slice_origin;
    }

    template <typename DstData, typename SrcData>
    __device__ static auto load_data(const SrcData* p_src, index_t src_offset)
    {
        return *reinterpret_cast<const DstData*>(&p_src[src_offset]);
    }

    template <typename DstData, typename SrcData>
    __device__ static void store_data(const SrcData src_data, DstData* p_dst, index_t dst_offset)
    {
        *reinterpret_cast<SrcData*>(&p_dst[dst_offset]) = src_data;
    }

    template <typename SrcData, index_t SrcDataPerAccess>
    struct vector_data_load;

    template <>
    struct vector_data_load<float, 1>
    {
        template <typename SrcCoord>
        __device__ static auto run(const float* p_src, const SrcCoord src_coord_begin)
        {
            return load_data<float>(p_src, src_coord_begin.GetOffset());
        }
    };

    template <>
    struct vector_data_load<float, 2>
    {
        template <typename SrcCoord>
        __device__ static auto run(const float* p_src, const SrcCoord src_coord_begin)
        {
            return load_data<float2_t>(p_src, src_coord_begin.GetOffset());
        }
    };

    template <>
    struct vector_data_load<float, 4>
    {
        template <typename SrcCoord>
        __device__ static auto run(const float* p_src, const SrcCoord src_coord_begin)
        {
            return load_data<float4_t>(p_src, src_coord_begin.GetOffset());
        }
    };

    template <index_t SrcDataPerAccess, index_t SrcDataRange, typename SrcData, typename SrcCoord>
    __device__ static auto buffer_vector_load(const SrcData* p_src, const SrcCoord src_coord_begin)
    {
        auto src_offset = src_coord_begin.GetOffset();
        return amd_buffer_load<SrcData, SrcDataPerAccess>(p_src, src_offset, true, SrcDataRange);
    }

    template <typename DstData, index_t DstDataPerAccess>
    struct vector_data_store;

    template <>
    struct vector_data_store<float, 1>
    {
        template <typename DstCoord>
        __device__ static void
        run(float* p_dst, const float src_data, const DstCoord dst_coord_begin)
        {
            store_data(src_data, p_dst, dst_coord_begin.GetOffset());
        }
    };

    template <>
    struct vector_data_store<float, 2>
    {
        template <typename DstCoord>
        __device__ static void
        run(float* p_dst, const float2_t src_data, const DstCoord dst_coord_begin)
        {
            store_data(src_data, p_dst, dst_coord_begin.GetOffset());
        }
    };

    template <>
    struct vector_data_store<float, 4>
    {
        template <typename DstCoord>
        __device__ static void
        run(float* p_dst, const float4_t src_data, const DstCoord dst_coord_begin)
        {
            store_data(src_data, p_dst, dst_coord_begin.GetOffset());
        }
    };

    template <typename SrcData, typename DstData>
    __device__ void Load(const SrcData* p_src, DstData& thread_buff)
    {
        constexpr auto vector_access_dim = Number<SrcVectorReadDim>{};

        constexpr auto src_data_per_access = Number<SrcDataPerRead>{};

        static_assert(SrcDataPerRead == 1 || SrcDataPerRead == 2 || SrcDataPerRead == 4, "");

        constexpr auto long_vector_size = src_data_per_access;

        constexpr auto long_vector_access_lengths = SliceLengths::Modify(
            vector_access_dim, SliceLengths::Get(vector_access_dim) / long_vector_size);

        static_ford<decltype(long_vector_access_lengths), SrcDimAccessOrder>{}(
            [&](auto long_vector_access_id) {
                constexpr auto long_vector_data_begin_id = long_vector_access_id.Modify(
                    Number<vector_access_dim>{},
                    Number<long_vector_size * long_vector_access_id[vector_access_dim]>{});

                // load data from src to the long-vector buffer
                const auto src_coord = mSrcSliceOrigin + to_multi_index(long_vector_data_begin_id);

                auto src_buff = GetRegBuffer<SrcData, SrcDataPerRead>();
                src_buff.GetVector(Number<SrcDataPerRead>{})(Number<0>{}) =
                    buffer_vector_load<SrcDataPerRead, SrcDesc::GetElementSpace()>(p_src,
                                                                                   src_coord);

                static_for<0, SrcDataPerRead, 1>{}([&](auto i) {
                    constexpr auto vector_id = long_vector_data_begin_id.Modify(
                        Number<vector_access_dim>{}, long_vector_access_id[vector_access_dim] + i);

                    // store data from the long-vector buffer to dst
                    constexpr auto buff_off =
                        ThreadBufferDesc::CalculateOffset(to_multi_index(vector_id));

                    thread_buff.GetVector(Number<1>{})(Number<buff_off>{}) =
                        src_buff.GetVector(Number<1>{})(Number<i>{});
                });
            });
    }

    template <typename SrcData, typename DstData>
    __device__ void Store(SrcData thread_buff, DstData* p_dst)
    {
        constexpr auto vector_access_dim = Number<DstVectorWriteDim>{};

        constexpr auto dst_data_per_access = Number<DstDataPerWrite>{};

        static_assert(DstDataPerWrite == 1 || DstDataPerWrite == 2 || DstDataPerWrite == 4, "");

        constexpr auto long_vector_size = dst_data_per_access;

        constexpr auto long_vector_access_lengths = SliceLengths::Modify(
            vector_access_dim, SliceLengths::Get(vector_access_dim) / long_vector_size);

        static_ford<decltype(long_vector_access_lengths), DstDimAccessOrder>{}(
            [&](auto long_vector_access_id) {
                constexpr auto long_vector_data_begin_id = long_vector_access_id.Modify(
                    Number<vector_access_dim>{},
                    Number<long_vector_size * long_vector_access_id[vector_access_dim]>{});

                auto src_buff = GetRegBuffer<DstData, DstDataPerWrite>();

                static_for<0, DstDataPerWrite, 1>{}([&](auto i) {
                    constexpr auto vector_id = long_vector_data_begin_id.Modify(
                        Number<vector_access_dim>{}, long_vector_access_id[vector_access_dim] + i);

                    constexpr auto buff_off =
                        ThreadBufferDesc::CalculateOffset(to_multi_index(vector_id));

                    src_buff.GetVector(Number<1>{})(Number<i>{}) =
                        thread_buff.GetVector(Number<1>{})[Number<buff_off>{}];
                });

                const auto dst_coord = mDstSliceOrigin + to_multi_index(long_vector_data_begin_id);

                vector_data_store<DstData, DstDataPerWrite>::run(
                    p_dst, src_buff.GetVector(Number<DstDataPerWrite>{})[Number<0>{}], dst_coord);
            });
    }

    template <typename T, bool PositiveDirection>
    __device__ void MoveSrcSliceWindow(const T& step_sizes_,
                                       integral_constant<bool, PositiveDirection>)
    {
        const auto step_sizes = to_multi_index(step_sizes_);

        static_if<PositiveDirection>{}([&](auto) { mSrcSliceOrigin += to_multi_index(step_sizes); })
            .Else([&](auto) { mSrcSliceOrigin -= step_sizes; });
    }

    template <typename T, bool PositiveDirection>
    __device__ void MoveDstSliceWindow(const T& step_sizes_,
                                       integral_constant<bool, PositiveDirection>)
    {
        const auto step_sizes = to_multi_index(step_sizes_);

        static_if<PositiveDirection>{}([&](auto) { mDstSliceOrigin += step_sizes; })
            .Else([&](auto) { mDstSliceOrigin -= step_sizes; });
    }

    private:
    SrcCoord mSrcSliceOrigin;
    DstCoord mDstSliceOrigin;
};

} // namespace ck
#endif
