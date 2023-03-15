// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2022, Advanced Micro Devices, Inc. All rights reserved.

#include "gemm_quantization_common.hpp"
#include "ck/tensor_operation/gpu/device/impl/device_gemm_multiple_d_xdl_cshuffle.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace instance {

template <typename OutElementOp, LoopScheduler GemmLoopScheduler, PipelineVersion GemmPipeline>
using device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_km_kn_mn_instances = std::tuple<
    // clang-format off
        //##############################|      A|      B|          Ds|      E|  AData|  BData| AccData| CShuffle|      DsData|   EData|           A|           B|           CDE|           GEMM| NumGemmK| Block|  MPer|  NPer|  KPer| AK1| BK1| MPer| NPer| MXdl| NXdl|  ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockLds|  BBlockTransfer| BBlockTransfer| BBlockTransfer| BlockTransfer| BBlockTransfer| BBlockTransfer| BBlockLds|    CShuffle|    CShuffle| CBlockTransferClusterLengths|  CBlockTransfer|     LoopScheduler|     Pipeline|
        //##############################| Layout| Layout|      Layout| Layout|   Type|   Type|    Type| DataType|        Type|    Type| Elementwise| Elementwise|   Elementwise| Specialization| Prefetch|  Size| Block| Block| Block|    |    |  XDL|  XDL|  Per|  Per|   ThreadCluster|  ThreadCluster| SrcAccessOrder|   SrcVectorDim|      SrcScalar|      DstScalar| AddExtraM|   ThreadCluster|  ThreadCluster| SrcAccessOrder|  SrcVectorDim|      SrcScalar|      DstScalar| AddExtraN| MXdlPerWave| NXdlPerWave|         _MBlock_MWaveMPerXdl| ScalarPerVector|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |   Operation|   Operation|     Operation|               |    Stage|      |      |      |      |    |    |     |     | Wave| Wave| Lengths_K0_M_K1|   ArrangeOrder|               |               |      PerVector|   PerVector_K1|          | Lengths_K0_N_K1|   ArrangeOrder|               |              |      PerVector|   PerVector_K1|          |  PerShuffle|  PerShuffle|         _NBlock_NWaveNPerXdl|   _NWaveNPerXdl|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |            |            |              |               |         |      |      |      |      |    |    |     |     |     |     |                |               |               |               |               |               |          |                |               |               |              |               |               |          |            |            |                             |                |                  |             |
        DeviceGemmMultipleD_Xdl_CShuffle<    Col,    Row, Empty_Tuple,    Row, int8_t, int8_t, int32_t,  int32_t, Empty_Tuple,  int8_t, PassThrough, PassThrough,  OutElementOp,     MNKPadding,        1,   256,   256,   128,    64,   4,   4,   32,   32,    4,    2,     S<4, 64, 1>,     S<0, 2, 1>,     S<0, 2, 1>,              1,              4,              4,         0,     S<8, 32, 1>,     S<0, 2, 1>,     S<0, 2, 1>,             1,              4,              4,         0,           1,           1,               S<1, 64, 1, 4>,              16, GemmLoopScheduler, GemmPipeline>
    // clang-format on
    >;

template <typename OutElementOp, LoopScheduler GemmLoopScheduler, PipelineVersion GemmPipeline>
using device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_km_nk_mn_instances = std::tuple<
    // clang-format off
        //##############################|      A|      B|          Ds|      E|  AData|  BData| AccData| CShuffle|      DsData|   EData|           A|           B|           CDE|           GEMM| NumGemmK| Block|  MPer|  NPer|  KPer| AK1| BK1| MPer| NPer| MXdl| NXdl|  ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockLds|  BBlockTransfer| BBlockTransfer| BBlockTransfer| BlockTransfer| BBlockTransfer| BBlockTransfer| BBlockLds|    CShuffle|    CShuffle| CBlockTransferClusterLengths|  CBlockTransfer|     LoopScheduler|     Pipeline|
        //##############################| Layout| Layout|      Layout| Layout|   Type|   Type|    Type| DataType|        Type|    Type| Elementwise| Elementwise|   Elementwise| Specialization| Prefetch|  Size| Block| Block| Block|    |    |  XDL|  XDL|  Per|  Per|   ThreadCluster|  ThreadCluster| SrcAccessOrder|   SrcVectorDim|      SrcScalar|      DstScalar| AddExtraM|   ThreadCluster|  ThreadCluster| SrcAccessOrder|  SrcVectorDim|      SrcScalar|      DstScalar| AddExtraN| MXdlPerWave| NXdlPerWave|         _MBlock_MWaveMPerXdl| ScalarPerVector|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |   Operation|   Operation|     Operation|               |    Stage|      |      |      |      |    |    |     |     | Wave| Wave| Lengths_K0_M_K1|   ArrangeOrder|               |               |      PerVector|   PerVector_K1|          | Lengths_K0_N_K1|   ArrangeOrder|               |              |      PerVector|   PerVector_K1|          |  PerShuffle|  PerShuffle|         _NBlock_NWaveNPerXdl|   _NWaveNPerXdl|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |            |            |              |               |         |      |      |      |      |    |    |     |     |     |     |                |               |               |               |               |               |          |                |               |               |              |               |               |          |            |            |                             |                |                  |             |
        DeviceGemmMultipleD_Xdl_CShuffle<    Col,    Col, Empty_Tuple,    Row, int8_t, int8_t, int32_t,  int32_t, Empty_Tuple,  int8_t, PassThrough, PassThrough,  OutElementOp,     MNKPadding,        1,   256,   256,   128,    64,   4,  16,   32,   32,    4,    2,     S<4, 64, 1>,     S<0, 2, 1>,     S<0, 2, 1>,              1,              4,              4,         0,     S<4, 64, 1>,     S<1, 0, 2>,     S<1, 0, 2>,             2,             16,             16,         1,           1,           1,               S<1, 64, 1, 4>,              16, GemmLoopScheduler, GemmPipeline>
    // clang-format on
    >;

template <typename OutElementOp, LoopScheduler GemmLoopScheduler, PipelineVersion GemmPipeline>
using device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_mk_kn_mn_instances = std::tuple<
    // clang-format off
        //##############################|      A|      B|          Ds|      E|  AData|  BData| AccData| CShuffle|      DsData|   EData|           A|           B|           CDE|           GEMM| NumGemmK| Block|  MPer|  NPer|  KPer| AK1| BK1| MPer| NPer| MXdl| NXdl|  ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockLds|  BBlockTransfer| BBlockTransfer| BBlockTransfer| BlockTransfer| BBlockTransfer| BBlockTransfer| BBlockLds|    CShuffle|    CShuffle| CBlockTransferClusterLengths|  CBlockTransfer|     LoopScheduler|     Pipeline|
        //##############################| Layout| Layout|      Layout| Layout|   Type|   Type|    Type| DataType|        Type|    Type| Elementwise| Elementwise|   Elementwise| Specialization| Prefetch|  Size| Block| Block| Block|    |    |  XDL|  XDL|  Per|  Per|   ThreadCluster|  ThreadCluster| SrcAccessOrder|   SrcVectorDim|      SrcScalar|      DstScalar| AddExtraM|   ThreadCluster|  ThreadCluster| SrcAccessOrder|  SrcVectorDim|      SrcScalar|      DstScalar| AddExtraN| MXdlPerWave| NXdlPerWave|         _MBlock_MWaveMPerXdl| ScalarPerVector|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |   Operation|   Operation|     Operation|               |    Stage|      |      |      |      |    |    |     |     | Wave| Wave| Lengths_K0_M_K1|   ArrangeOrder|               |               |      PerVector|   PerVector_K1|          | Lengths_K0_N_K1|   ArrangeOrder|               |              |      PerVector|   PerVector_K1|          |  PerShuffle|  PerShuffle|         _NBlock_NWaveNPerXdl|   _NWaveNPerXdl|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |            |            |              |               |         |      |      |      |      |    |    |     |     |     |     |                |               |               |               |               |               |          |                |               |               |              |               |               |          |            |            |                             |                |                  |             |
        DeviceGemmMultipleD_Xdl_CShuffle<    Row,    Row, Empty_Tuple,    Row, int8_t, int8_t, int32_t,  int32_t, Empty_Tuple,  int8_t, PassThrough, PassThrough,  OutElementOp,     MNKPadding,        1,   256,   256,   128,    64,  16,   4,   32,   32,    4,    2,     S<4, 64, 1>,     S<1, 0, 2>,     S<1, 0, 2>,              2,             16,             16,         1,     S<8, 32, 1>,     S<0, 2, 1>,     S<0, 2, 1>,             1,              4,              4,         0,           1,           1,               S<1, 64, 1, 4>,              16, GemmLoopScheduler, GemmPipeline>
    // clang-format on
    >;

template <typename OutElementOp, LoopScheduler GemmLoopScheduler, PipelineVersion GemmPipeline>
using device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_mk_nk_mn_instances = std::tuple<
    // clang-format off
        //##############################|      A|      B|          Ds|      E|  AData|  BData| AccData| CShuffle|      DsData|   EData|           A|           B|           CDE|           GEMM| NumGemmK| Block|  MPer|  NPer|  KPer| AK1| BK1| MPer| NPer| MXdl| NXdl|  ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockLds|  BBlockTransfer| BBlockTransfer| BBlockTransfer| BlockTransfer| BBlockTransfer| BBlockTransfer| BBlockLds|    CShuffle|    CShuffle| CBlockTransferClusterLengths|  CBlockTransfer|     LoopScheduler|     Pipeline|
        //##############################| Layout| Layout|      Layout| Layout|   Type|   Type|    Type| DataType|        Type|    Type| Elementwise| Elementwise|   Elementwise| Specialization| Prefetch|  Size| Block| Block| Block|    |    |  XDL|  XDL|  Per|  Per|   ThreadCluster|  ThreadCluster| SrcAccessOrder|   SrcVectorDim|      SrcScalar|      DstScalar| AddExtraM|   ThreadCluster|  ThreadCluster| SrcAccessOrder|  SrcVectorDim|      SrcScalar|      DstScalar| AddExtraN| MXdlPerWave| NXdlPerWave|         _MBlock_MWaveMPerXdl| ScalarPerVector|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |   Operation|   Operation|     Operation|               |    Stage|      |      |      |      |    |    |     |     | Wave| Wave| Lengths_K0_M_K1|   ArrangeOrder|               |               |      PerVector|   PerVector_K1|          | Lengths_K0_N_K1|   ArrangeOrder|               |              |      PerVector|   PerVector_K1|          |  PerShuffle|  PerShuffle|         _NBlock_NWaveNPerXdl|   _NWaveNPerXdl|                  |             |
        //##############################|       |       |            |       |       |       |        |         |            |        |            |            |              |               |         |      |      |      |      |    |    |     |     |     |     |                |               |               |               |               |               |          |                |               |               |              |               |               |          |            |            |                             |                |                  |             |
        DeviceGemmMultipleD_Xdl_CShuffle<    Row,    Col, Empty_Tuple,    Row, int8_t, int8_t, int32_t,  int32_t, Empty_Tuple,  int8_t, PassThrough, PassThrough,  OutElementOp,     MNKPadding,        1,   256,   256,   128,    64,  16,  16,   32,   32,    4,    2,     S<4, 64, 1>,     S<1, 0, 2>,     S<1, 0, 2>,              2,             16,             16,         1,     S<4, 64, 1>,     S<1, 0, 2>,     S<1, 0, 2>,             2,              8,              8,         1,           1,           1,               S<1, 32, 1, 4>,              16, GemmLoopScheduler, GemmPipeline>
    // clang-format on
    >;

// Layout(A, B, C) = [Col, Row, Row]
void add_device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_km_kn_mn_instances(
    std::vector<std::unique_ptr<DeviceGemmMultipleD<Col,
                                                    Row,
                                                    Empty_Tuple,
                                                    Row,
                                                    int8_t,
                                                    int8_t,
                                                    Empty_Tuple,
                                                    int8_t,
                                                    PassThrough,
                                                    PassThrough,
                                                    Mul_Clamp>>>& instances)
{
    add_device_operation_instances(
        instances,
        device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_km_kn_mn_instances<Mul_Clamp,
                                                                           LoopScheduler::Default,
                                                                           PipelineVersion::v1>{});
}

// Layout(A, B, C) = [Col, Col, Row]
void add_device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_km_nk_mn_instances(
    std::vector<std::unique_ptr<DeviceGemmMultipleD<Col,
                                                    Col,
                                                    Empty_Tuple,
                                                    Row,
                                                    int8_t,
                                                    int8_t,
                                                    Empty_Tuple,
                                                    int8_t,
                                                    PassThrough,
                                                    PassThrough,
                                                    Mul_Clamp>>>& instances)
{
    add_device_operation_instances(
        instances,
        device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_km_nk_mn_instances<Mul_Clamp,
                                                                           LoopScheduler::Default,
                                                                           PipelineVersion::v1>{});
}

// Layout(A, B, C) = [Row, Row, Row]
void add_device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_mk_kn_mn_instances(
    std::vector<std::unique_ptr<DeviceGemmMultipleD<Row,
                                                    Row,
                                                    Empty_Tuple,
                                                    Row,
                                                    int8_t,
                                                    int8_t,
                                                    Empty_Tuple,
                                                    int8_t,
                                                    PassThrough,
                                                    PassThrough,
                                                    Mul_Clamp>>>& instances)
{
    add_device_operation_instances(
        instances,
        device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_mk_kn_mn_instances<Mul_Clamp,
                                                                           LoopScheduler::Default,
                                                                           PipelineVersion::v1>{});
}

// Layout(A, B, C) = [Row, Col, Row]
void add_device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_mk_nk_mn_instances(
    std::vector<std::unique_ptr<DeviceGemmMultipleD<Row,
                                                    Col,
                                                    Empty_Tuple,
                                                    Row,
                                                    int8_t,
                                                    int8_t,
                                                    Empty_Tuple,
                                                    int8_t,
                                                    PassThrough,
                                                    PassThrough,
                                                    Mul_Clamp>>>& instances)
{
    add_device_operation_instances(
        instances,
        device_gemm_quantization_xdl_c_shuffle_i8_i8_i8_mk_nk_mn_instances<Mul_Clamp,
                                                                           LoopScheduler::Default,
                                                                           PipelineVersion::v1>{});
}

} // namespace instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
