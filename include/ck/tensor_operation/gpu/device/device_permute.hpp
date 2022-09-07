// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2022, Advanced Micro Devices, Inc. All rights reserved.

#pragma once

#include <array>
#include <memory>
#include <utility>

#include "ck/utility/math.hpp"
#include "ck/utility/sequence.hpp"
#include "ck/tensor_operation/gpu/device/device_base.hpp"
#include "ck/tensor_operation/gpu/device/matrix_padder.hpp"
#include "ck/tensor_operation/gpu/grid/gridwise_copy.hpp"
#include "ck/tensor_description/tensor_descriptor_helper.hpp"

#include "ck/host_utility/kernel_launch.hpp"

namespace ck {
namespace tensor_operation {
namespace device {

namespace detail {
template <typename Derived>
struct DevicePermuteBase : BaseOperator
{
    bool IsSupportedArgument(const BaseArgument* arg) override final
    {
        const auto* argument = dynamic_cast<const typename Derived::Argument*>(arg);
        if(!argument)
        {
            return false;
        }

        return Derived::IsSupportedArgument(*argument);
    }

    template <typename... Args>
    static auto MakeArgument(Args&&... args)
    {
        return typename Derived::Argument{std::forward<Args>(args)...};
    }

    template <typename... Args>
    static auto MakeArgumentPointer(Args&&... args)
    {
        return std::make_unique<typename Derived::Argument>(std::forward<Args>(args)...);
    }

    static auto MakeInvoker() { return typename Derived::Invoker{}; }

    static auto MakeInvokerPointer() { return std::make_unique<typename Derived::Invoker>(); };
};

template <typename Derived, typename Argument>
struct InvokerBase : BaseInvoker
{
    float Run(const BaseArgument* arg,
              const StreamConfig& stream_config = StreamConfig{}) override final
    {
        const auto* argument = dynamic_cast<const Argument*>(arg);
        if(!argument)
        {
            return 0.f;
        }

        return Derived::Run(*argument, stream_config);
    }
};
} // namespace detail

// Swap last 2 dimensions
// input shape: [d[0], d[1], d[2], ..., d[NumDim-3], d[NumDim-2], d[NumDim-1]]
//                                                                ^^^^^^^^^^^
// output shape: [d[0], d[1], d[2], ..., d[NumDim-3], d[NumDim-1], d[NumDim-2]]
//                                                    ^^^^^^^^^^^
template <typename InDataType,
          typename OutDataType,
          typename ElementwiseOperation,
          index_t NumDim,
          index_t BlockSize,
          index_t NPerBlock,
          index_t HPerBlock,
          index_t WPerBlock,
          index_t MPerThread,
          index_t InScalarPerVector,
          index_t OutScalarPerVector>
struct DevicePermute : detail::DevicePermuteBase<DevicePermute<InDataType,
                                                               OutDataType,
                                                               ElementwiseOperation,
                                                               NumDim,
                                                               BlockSize,
                                                               NPerBlock,
                                                               HPerBlock,
                                                               WPerBlock,
                                                               MPerThread,
                                                               InScalarPerVector,
                                                               OutScalarPerVector>>
{
    static_assert(3 <= NumDim, "Only accept at least 3D dimension tensor");

    using InDataTypePointer  = const InDataType*;
    using OutDataTypePointer = OutDataType*;

    template <index_t N = NumDim>
    static auto ConvertArrayToTuple(const std::array<index_t, NumDim>& array)
    {
        static_assert(1 <= N && N <= NumDim);

        return generate_tuple([&](auto I) { return array[I]; }, Number<N>{});
    }

    static auto MakeDescriptor_N_H_W(const std::array<index_t, NumDim>& lengths,
                                     const std::array<index_t, NumDim>& stride)
    {
        // create nd descriptor, shape: [d[0], d[1], d[2], ..., d[NumDim-3], d[NumDim-2],
        // d[NumDim-1]]
        const auto desc =
            make_naive_tensor_descriptor(ConvertArrayToTuple(lengths), ConvertArrayToTuple(stride));

        // merge nd to 3d descriptor, shape: [(d[0] * d[1] * d[2] * ... * d[NumDim-3]), d[NumDim-2],
        // d[NumDim-1]]
        //                                   => [N, H, W]
        const index_t H       = *std::next(rbegin(lengths));
        const index_t W       = *rbegin(lengths);
        const auto desc_n_h_w = transform_tensor_descriptor(
            desc,
            make_tuple(make_merge_transform(ConvertArrayToTuple<NumDim - 2>(lengths)),
                       make_pass_through_transform(H),
                       make_pass_through_transform(W)),
            make_tuple(generate_sequence_v2([&](auto I) { return I; }, Number<NumDim - 2>{}),
                       Sequence<NumDim - 2>{},
                       Sequence<NumDim - 1>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}));

        return PadTensorDescriptor(
            desc_n_h_w, make_tuple(NPerBlock, HPerBlock, WPerBlock), Sequence<true, true, true>{});
    }

    using InGrid1dDesc  = decltype(MakeDescriptor_N_H_W({1, 1}, {1, 1}));
    using OutGrid1dDesc = decltype(MakeDescriptor_N_H_W({1, 1}, {1, 1}));

    using GridwiseCopy = GridwiseCopy<InGrid1dDesc,
                                      OutGrid1dDesc,
                                      InDataTypePointer,
                                      OutDataTypePointer,
                                      ElementwiseOperation,
                                      BlockSize,
                                      NPerBlock,
                                      HPerBlock,
                                      WPerBlock,
                                      MPerThread,
                                      InScalarPerVector,
                                      OutScalarPerVector>;

    struct Argument : public BaseArgument
    {
        Argument(const std::array<index_t, NumDim> inLengths,
                 const std::array<index_t, NumDim> inStrides,
                 const std::array<index_t, NumDim> outLengths,
                 const std::array<index_t, NumDim> outStrides,
                 const void* in_dev_buffer,
                 void* out_dev_buffer,
                 ElementwiseOperation elementwise_op)
            : in_dev_buffer_(static_cast<InDataTypePointer>(in_dev_buffer)),
              out_dev_buffer_(static_cast<OutDataTypePointer>(out_dev_buffer)),
              in_grid_1d_desc_(MakeDescriptor_N_H_W(inLengths, inStrides)),
              out_grid_1d_desc_(MakeDescriptor_N_H_W(inLengths, inStrides)),
              inLengths_(inLengths),
              inStrides_(inStrides),
              outLengths_(outLengths),
              outStrides_(outStrides),
              elementwise_op_(elementwise_op),
              block_2_tile_map_(GridwiseCopy::MakeDefaultBlock2TileMap(in_grid_1d_desc_))
        {
        }

        InDataTypePointer in_dev_buffer_;
        OutDataTypePointer out_dev_buffer_;
        InGrid1dDesc in_grid_1d_desc_;
        OutGrid1dDesc out_grid_1d_desc_;

        std::array<index_t, NumDim> inLengths_;
        std::array<index_t, NumDim> inStrides_;
        std::array<index_t, NumDim> outLengths_;
        std::array<index_t, NumDim> outStrides_;

        ElementwiseOperation elementwise_op_;

        typename GridwiseCopy::DefaultBlock2TileMap block_2_tile_map_;
    };

    struct Invoker : detail::InvokerBase<Invoker, Argument>
    {
        static float Run(const Argument& arg, const StreamConfig& stream_config = StreamConfig{})
        {
            const index_t grid_size = arg.block_2_tile_map_.CalculateGridSize(arg.in_grid_1d_desc_);

            const auto kernel = kernel_nd_copy<GridwiseCopy,
                                               InGrid1dDesc,
                                               OutGrid1dDesc,
                                               InDataTypePointer,
                                               OutDataTypePointer,
                                               ElementwiseOperation,
                                               typename GridwiseCopy::DefaultBlock2TileMap>;

            float elapsed_time = launch_and_time_kernel(stream_config,
                                                        kernel,
                                                        dim3(grid_size),
                                                        dim3(BlockSize),
                                                        0,
                                                        arg.in_grid_1d_desc_,
                                                        arg.out_grid_1d_desc_,
                                                        arg.in_dev_buffer_,
                                                        arg.out_dev_buffer_,
                                                        arg.elementwise_op_,
                                                        arg.block_2_tile_map_);
            return elapsed_time;
        }
    };

    static bool IsSupportedArgument(const Argument& arg)
    {
        if(arg.inLengths_.back() % MPerThread != 0)
        {
            return false;
        }

        // check if only swap last 2 dimensions
        if(!(std::equal(begin(arg.inLengths_),
                        std::prev(end(arg.inLengths_), 2),
                        begin(arg.outLengths_)) &&
             std::tie(*rbegin(arg.inLengths_), *std::next(rbegin(arg.inLengths_))) ==
                 std::tie(*std::next(rbegin(arg.outLengths_)), *rbegin(arg.outLengths_))))
        {
            return false;
        }

        auto IsScalarPerVectorValid = [&](const std::array<index_t, NumDim>& lengths,
                                          const std::array<index_t, NumDim>& strides,
                                          index_t scalarPerVector) {
            if(strides.back() == 1 && lengths.back() % scalarPerVector == 0)
                return true;

            if(strides.back() != 1 && scalarPerVector == 1)
                return true;

            return false;
        };

        bool valid = true;
        if(!IsScalarPerVectorValid(arg.inLengths_, arg.inStrides_, InScalarPerVector))
        {
            valid = false;
        }

        if(!IsScalarPerVectorValid(arg.outLengths_, arg.outStrides_, OutScalarPerVector))
        {
            valid = false;
        }

        return valid;
    };
};

} // namespace device
} // namespace tensor_operation
} // namespace ck
