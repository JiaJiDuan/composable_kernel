// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2023, Advanced Micro Devices, Inc. All rights reserved.

#include "ck/library/tensor_operation_instance/add_device_operation_instance.hpp"
#include "ck/library/tensor_operation_instance/gpu/grouped_conv_fwd/device_grouped_conv_fwd_xdl_instance.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace instance {
// Compilation parameters for in[g, n, hi, wi, c] * wei[g, k, y, x, c] = out[g, n, ho, wo, k]
void add_device_grouped_conv2d_fwd_xdl_gnhwc_gkyxc_gnhwk_f16_instances(
    std::vector<std::unique_ptr<DeviceGroupedConvFwdMultipleD<2,
                                                              GNHWC,
                                                              GKYXC,
                                                              Empty_Tuple,
                                                              GNHWK,
                                                              F16,
                                                              F16,
                                                              Empty_Tuple,
                                                              F16,
                                                              PassThrough,
                                                              PassThrough,
                                                              PassThrough>>>& instances)
{
    add_device_operation_instances(instances,
                                   device_grouped_conv_fwd_xdl_f16_instances<2,
                                                                             GNHWC,
                                                                             GKYXC,
                                                                             Empty_Tuple,
                                                                             GNHWK,
                                                                             ConvFwdDefault>{});

    add_device_operation_instances(instances,
                                   device_grouped_conv_fwd_xdl_f16_instances<2,
                                                                             GNHWC,
                                                                             GKYXC,
                                                                             Empty_Tuple,
                                                                             GNHWK,
                                                                             ConvFwd1x1P0>{});

    add_device_operation_instances(instances,
                                   device_grouped_conv_fwd_xdl_f16_instances<2,
                                                                             GNHWC,
                                                                             GKYXC,
                                                                             Empty_Tuple,
                                                                             GNHWK,
                                                                             ConvFwd1x1S1P0>{});

    add_device_operation_instances(instances,
                                   device_grouped_conv_fwd_xdl_f16_instances<2,
                                                                             GNHWC,
                                                                             GKYXC,
                                                                             Empty_Tuple,
                                                                             GNHWK,
                                                                             ConvFwdOddC>{});
}

} // namespace instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
