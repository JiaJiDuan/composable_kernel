// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2023, Advanced Micro Devices, Inc. All rights reserved.

#include "ck/library/tensor_operation_instance/add_device_operation_instance.hpp"
#include "ck/library/tensor_operation_instance/gpu/grouped_conv_bwd_data/device_grouped_conv_bwd_data_xdl_instance.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace instance {
// Compilation parameters for out[g, n, di, hi, wi, c] * wei[g, k, z, y, x, c] = in[g, n, do, ho,
// wo, k]
void add_device_grouped_conv3d_bwd_data_xdl_gndhwc_gkzyxc_gndhwk_bf16_instances(
    std::vector<std::unique_ptr<DeviceGroupedConvBwdDataMultipleD<3,
                                                                  GNDHWK,
                                                                  GKZYXC,
                                                                  Empty_Tuple,
                                                                  GNDHWC,
                                                                  BF16,
                                                                  BF16,
                                                                  Empty_Tuple,
                                                                  BF16,
                                                                  PassThrough,
                                                                  PassThrough,
                                                                  PassThrough>>>& instances)
{
    // 1. Default
    add_device_operation_instances(
        instances,
        device_grouped_conv_bwd_data_xdl_bf16_instances<3,
                                                        GNDHWK,
                                                        GKZYXC,
                                                        Empty_Tuple,
                                                        GNDHWC,
                                                        ConvBwdDataDefault>{});
    // 2. Filter1x1Stride1Pad0
    add_device_operation_instances(
        instances,
        device_grouped_conv_bwd_data_xdl_bf16_instances<3,
                                                        GNDHWK,
                                                        GKZYXC,
                                                        Empty_Tuple,
                                                        GNDHWC,
                                                        ConvBwdDataFilter1x1Stride1Pad0>{});
}

} // namespace instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
