// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2023, Advanced Micro Devices, Inc. All rights reserved.

#pragma once
#ifdef __fp16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f16_f16_f16_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f16_f16_f16_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f16_f16_f16_amax.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f16_f32_f16_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f16_f32_f16_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f16_f32_f16_norm2.hpp"
#endif
#ifdef __fp16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f32_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f32_f32_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f32_f32_norm2.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f32_f32_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f32_f32_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f32_f32_amax.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f64_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f64_f32_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f32_f64_f32_norm2.hpp"
#endif
#ifdef __fp64__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f64_f64_f64_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f64_f64_f64_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f64_f64_f64_norm2.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f64_f64_f64_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f64_f64_f64_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_f64_f64_f64_amax.hpp"
#endif
#ifdef __int8__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_i8_i8_i8_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_i8_i8_i8_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_i8_i8_i8_amax.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_i8_i32_i8_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_i8_i32_i8_avg.hpp"
#endif
#ifdef __bf16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_b16_f32_b16_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_b16_f32_b16_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_b16_f32_b16_norm2.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_b16_f32_b16_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_b16_f32_b16_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_blockwise_b16_f32_b16_amax.hpp"
#endif
#ifdef __fp16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f16_f32_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f16_f32_f32_avg.hpp"
#endif
#ifdef __fp32__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f32_f32_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f32_f32_f32_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f32_f64_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f32_f64_f32_avg.hpp"
#endif
#ifdef __fp64__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f64_f64_f64_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_f64_f64_f64_avg.hpp"
#endif
#ifdef __bf16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_b16_f32_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_multiblock_atomic_add_b16_f32_f32_avg.hpp"
#endif
#ifdef __fp16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f16_f16_f16_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f16_f16_f16_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f16_f16_f16_amax.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f16_f32_f16_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f16_f32_f16_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f16_f32_f16_norm2.hpp"
#endif
#ifdef __fp32__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f32_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f32_f32_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f32_f32_norm2.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f32_f32_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f32_f32_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f32_f32_amax.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f64_f32_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f64_f32_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f32_f64_f32_norm2.hpp"
#endif
#ifdef __fp64__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f64_f64_f64_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f64_f64_f64_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f64_f64_f64_norm2.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f64_f64_f64_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f64_f64_f64_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_f64_f64_f64_amax.hpp"
#endif
#ifdef __int8__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_i8_i8_i8_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_i8_i8_i8_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_i8_i8_i8_amax.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_i8_i32_i8_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_i8_i32_i8_avg.hpp"
#endif
#ifdef __bf16__
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_b16_f32_b16_add.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_b16_f32_b16_avg.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_b16_f32_b16_norm2.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_b16_f32_b16_min.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_b16_f32_b16_max.hpp"
#include "ck/library/tensor_operation_instance/gpu/reduce/device_reduce_instance_threadwise_b16_f32_b16_amax.hpp"
#endif
