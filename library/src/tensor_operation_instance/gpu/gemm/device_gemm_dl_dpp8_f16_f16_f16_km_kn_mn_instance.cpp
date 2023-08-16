// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2023, Advanced Micro Devices, Inc. All rights reserved.

#include <cstdlib>

#include "ck/ck.hpp"
#include "ck/tensor_operation/gpu/device/tensor_layout.hpp"
#include "ck/tensor_operation/gpu/device/gemm_specialization.hpp"
#include "ck/tensor_operation/gpu/device/impl/device_gemm_dl_dpp8.hpp"
#include "ck/library/tensor_operation_instance/add_device_operation_instance.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace instance {

using F16 = ck::half_t;
using F32 = float;

using Row = ck::tensor_layout::gemm::RowMajor;
using Col = ck::tensor_layout::gemm::ColumnMajor;

template <ck::index_t... Is>
using S = ck::Sequence<Is...>;

using PassThrough = ck::tensor_operation::element_wise::PassThrough;

static constexpr auto GemmDefault = ck::tensor_operation::device::GemmSpecialization::Default;

// Compilation parameters for a[k, m] * b[k, n] = c[m, n]
using device_gemm_dl_dpp8_f16_f16_f16_km_kn_mn_instances = std::tuple<
    // clang-format off
        // ##########| AData| BData| CData| AccData| ALayout| BLayout| CLayout|           A|           B|           C|           GEMM| Block|  MPer|  NPer| K0Per| K1|      M1Per|      N1Per|   KPer|  M11N11Thread|  M11N11Thread|     ABlockTransfer|       ABlockTransfer| ABlockTransfer| ABlockTransfer|      ABlockTransfer|     ABlockTransfer|       ABlockTransfer|     BBlockTransfer|       BBlockTransfer| BBlockTransfer| BBlockTransfer|      BBlockTransfer|     BBlockTransfer|       BBlockTransfer|      CThreadTransfer|  CThreadTransfer|    CThreadTransfer|
        // ##########|  Type|  Type|  Type|    Type|        |        |        | Elementwise| Elementwise| Elementwise| Specialization|  Size| Block| Block| Block|   | ThreadM111| ThreadN111| Thread| ClusterM110Xs| ClusterN110Xs| ThreadSliceLengths| ThreadClusterLengths|  ThreadCluster|      SrcAccess|     SrcVectorTensor|    SrcVectorTensor|      DstVectorTensor| ThreadSliceLengths| ThreadClusterLengths|  ThreadCluster|      SrcAccess|     SrcVectorTensor|    SrcVectorTensor|      DstVectorTensor|         SrcDstAccess|  SrcDstVectorDim| DstScalarPerVector|
        // ##########|      |      |      |        |        |        |        |   Operation|   Operation|   Operation|               |      |      |      |      |   |           |           |       |              |              |        K0_M0_M1_K1|          K0_M0_M1_K1|   ArrangeOrder|          Order| Lengths_K0_M0_M1_K1| ContiguousDimOrder|  Lengths_K0_M0_M1_K1|        K0_M0_M1_K1|          K0_M0_M1_K1|   ArrangeOrder|          Order| Lengths_K0_M0_M1_K1| ContiguousDimOrder|  Lengths_K0_M0_M1_K1|                Order|                 |                   |
        // ##########|      |      |      |        |        |        |        |            |            |            |               |      |      |      |      |   |           |           |       |              |              |                   |                     |               |               |                    |                   |                     |                   |                     |               |               |                    |                   |                     |                     |                 |                   |
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,     8,     8,     8,     4,  2,          1,          8,      1,       S<1, 8>,       S<1, 1>,      S<1, 1, 4, 2>,       S<4, 1,  2, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 4, 2>,       S<1, 1, 4, 2>,        S<4, 1, 2, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,    32,     8,    64,    16,  2,          1,          8,      1,       S<1, 8>,       S<4, 1>,      S<1, 1, 4, 2>,      S<16, 1,  2, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 2, 2>,      S<16, 1, 2, 2>,       S<1, 1, 32, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 2, 1>,      S<0, 3, 1, 2>,        S<1, 1, 2, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,    32,     8,    64,    16,  2,          1,          8,      1,       S<1, 8>,       S<4, 1>,      S<1, 1, 4, 2>,      S<16, 1,  2, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 2, 2>,       S<4, 1, 8, 2>,        S<4, 1, 8, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,    64,     8,    64,    16,  2,          1,          8,      1,       S<1, 8>,       S<8, 1>,      S<1, 1, 2, 2>,      S<16, 1,  4, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 2, 1>,      S<0, 3, 1, 2>,       S<1, 1, 2, 2>,       S<4, 1, 4, 2>,       S<4, 1, 16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,    64,    64,    64,     8,  2,          4,          8,      1,       S<2, 8>,       S<4, 1>,      S<2, 1, 4, 2>,      S<4, 1,  16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 4, 2>,       S<2, 1, 4, 2>,       S<4, 1, 16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,    64,     8,    64,    16,  2,          8,          1,      1,       S<1, 1>,       S<8, 8>,      S<1, 1, 2, 2>,      S<16, 1,  4, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 2, 1>,      S<0, 3, 1, 2>,       S<1, 1, 2, 2>,       S<4, 1, 4, 2>,       S<4, 1, 16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  1>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,   128,    64,    64,    16,  2,          4,          8,      1,       S<2, 8>,       S<8, 1>,      S<2, 1, 4, 2>,      S<8, 1,  16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 4, 2>,       S<2, 1, 4, 2>,       S<8, 1, 16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,   128,   128,    64,    16,  2,          1,          8,      8,       S<4, 8>,       S<4, 1>,      S<4, 1, 4, 2>,      S<4, 1,  32, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 4, 2>,       S<2, 1, 4, 2>,       S<8, 1, 16, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,   256,   128,   128,     8,  2,          1,          8,      8,       S<8, 8>,       S<4, 1>,      S<1, 1, 4, 2>,      S<8, 1,  32, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 4, 2>,       S<1, 1, 4, 2>,       S<8, 1, 32, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>,
        DeviceGemmDlDpp8< F16,   F16,   F16,     F32,     Col,     Row,     Row, PassThrough, PassThrough, PassThrough,    GemmDefault,   256,   128,   128,    16,  2,          4,          8,      8,       S<2, 8>,      S<16, 1>,      S<2, 1, 4, 2>,      S<8, 1,  32, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,       S<1, 1, 4, 2>,       S<2, 1, 4, 2>,       S<8, 1, 32, 1>,  S<0, 3, 1, 2>,  S<0, 3, 1, 2>,       S<1, 1, 4, 1>,      S<0, 3, 1, 2>,        S<1, 1, 4, 2>,  S<0, 1, 2, 3, 4, 5>,                5,                  4>
    // clang-format on
    >;

void add_device_gemm_dl_dpp8_f16_f16_f16_km_kn_mn_instances(
    std::vector<std::unique_ptr<
        DeviceGemm<Col, Row, Row, F16, F16, F16, PassThrough, PassThrough, PassThrough>>>&
        instances)
{
    add_device_operation_instances(instances, device_gemm_dl_dpp8_f16_f16_f16_km_kn_mn_instances{});
}

} // namespace instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
