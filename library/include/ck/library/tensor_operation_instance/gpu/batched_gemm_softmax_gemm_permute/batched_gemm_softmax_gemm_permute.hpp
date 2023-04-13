// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2022, Advanced Micro Devices, Inc. All rights reserved.

#pragma once

#include <cstdlib>

#include "ck/ck.hpp"
#include "ck/tensor_operation/gpu/device/tensor_layout.hpp"
#include "ck/tensor_operation/gpu/device/device_batched_gemm_softmax_gemm_permute.hpp"
#include "ck/tensor_operation/gpu/element/element_wise_operation.hpp"

#include "ck/library/tensor_operation_instance/device_operation_instance_factory.hpp"
#include "ck/library/tensor_operation_instance/gpu/batched_gemm_softmax_gemm_permute/device_batched_gemm_multiple_d_softmax_gemm_permute_xdl_cshuffle_half_gmk_gnk_gno_gmo_instance.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace instance {

template <index_t NumDimG,
          index_t NumDimM,
          index_t NumDimN,
          index_t NumDimK,
          index_t NumDimO,
          typename ADataType,
          typename B0DataType,
          typename B1DataType,
          typename CDataType,
          typename Acc0BiasDataType,
          typename Acc1BiasDataType,
          typename AElementwiseOperation,
          typename B0ElementwiseOperation,
          typename C0DEElementwiseOperation,
          typename B1ElementwiseOperation,
          typename C1DEElementwiseOperation,
          MaskingSpecialization MaskingSpec>
void add_device_instances(
    std::vector<std::unique_ptr<DeviceBatchedGemmSoftmaxGemmPermute<NumDimG,
                                                                    NumDimM,
                                                                    NumDimN,
                                                                    NumDimK,
                                                                    NumDimO,
                                                                    ADataType,
                                                                    B0DataType,
                                                                    B1DataType,
                                                                    CDataType,
                                                                    Acc0BiasDataType,
                                                                    Acc1BiasDataType,
                                                                    AElementwiseOperation,
                                                                    B0ElementwiseOperation,
                                                                    C0DEElementwiseOperation,
                                                                    B1ElementwiseOperation,
                                                                    C1DEElementwiseOperation,
                                                                    MaskingSpec>>>& instances)
{
    add_device_operation_instances(instances,
                                   create_device_instances<NumDimG,
                                                           NumDimM,
                                                           NumDimN,
                                                           NumDimK,
                                                           NumDimO,
                                                           ADataType,
                                                           B0DataType,
                                                           B1DataType,
                                                           CDataType,
                                                           Acc0BiasDataType,
                                                           Acc1BiasDataType,
                                                           AElementwiseOperation,
                                                           B0ElementwiseOperation,
                                                           C0DEElementwiseOperation,
                                                           B1ElementwiseOperation,
                                                           C1DEElementwiseOperation,
                                                           MaskingSpec>());
}
} // namespace instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
