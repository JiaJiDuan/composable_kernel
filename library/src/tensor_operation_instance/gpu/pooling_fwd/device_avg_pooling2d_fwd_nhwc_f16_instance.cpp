// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2022, Advanced Micro Devices, Inc. All rights reserved.

#include "pooling_fwd_instance_common.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace instance {

static constexpr auto ReduceOpId = ck::ReduceTensorOp::AVG;

void add_device_avg_pooling2d_fwd_nhwc_f16_instances(
    std::vector<std::unique_ptr<DevicePoolFwd<4, 2, ReduceOpId, false>>>& instances)
{
    add_device_operation_instances(
        instances, device_pooling2d_fwd_nhwc_instances<F16, F16, F32, ReduceOpId, false>{});
}

} // namespace instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
