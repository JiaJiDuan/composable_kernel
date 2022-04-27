#include <stdlib.h>
#include "convolution_forward_specialization_cpu.hpp"
#include "config.hpp"
#include "device_convnd_fwd_avx2_nhwc_kyxc_nhwk.hpp"
#include "element_wise_operation_cpu.hpp"
#include "device_operation_instance.hpp"

namespace ck {
namespace tensor_operation {
namespace cpu {
namespace device {
namespace device_conv2d_fwd_avx2_instance {

using InType                           = float;
using WeiType                          = float;
using OutType                          = float;
using AccType                          = float;
using InLayout                         = ck::tensor_layout::gemm::RowMajor;    // NHWC
using WeiLayout                        = ck::tensor_layout::gemm::ColumnMajor; // KYXC
static constexpr bool NonTemporalStore = false;

using PT = ck::tensor_operation::cpu::element_wise::PassThrough;
using ThreadwiseGemmAvx2_MxN_4x24_Dispatch =
    ck::cpu::ThreadwiseGemmAvx2_MxN_4x24_Dispatch<InType,
                                                  WeiType,
                                                  OutType,
                                                  InLayout,
                                                  WeiLayout,
                                                  NonTemporalStore>;

static constexpr auto ConvFwdDefault =
    ck::tensor_operation::cpu::device::ConvolutionForwardSpecialization_t::Default;

static constexpr auto ConvFwd1x1P0 =
    ck::tensor_operation::cpu::device::ConvolutionForwardSpecialization_t::Filter1x1Pad0;

static constexpr auto ConvFwd1x1S1P0 =
    ck::tensor_operation::cpu::device::ConvolutionForwardSpecialization_t::Filter1x1Stride1Pad0;

static constexpr auto DefaultGemmKLoop =
    ck::tensor_operation::cpu::device::ConvolutionForwardGemmKSpecialization_t::DefaultGemmKLoop;
static constexpr auto GemmKLoopOverC =
    ck::tensor_operation::cpu::device::ConvolutionForwardGemmKSpecialization_t::NHWC_GemmKLoopOverC;

static constexpr auto LoopOver_MNK = ck::tensor_operation::cpu::device::LoopOver_MNK;
static constexpr auto LoopOver_MKN = ck::tensor_operation::cpu::device::LoopOver_MKN;

// clang-format off
#define DEVICE_CONV2D_FWD_AVX2_NHWC_KYXC_NHWK_F32(a_elem_op, b_elem_op, c_elem_op, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf) \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwdDefault, GemmKLoopOverC  , LoopOver_MNK, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwd1x1P0,   GemmKLoopOverC  , LoopOver_MNK, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwdDefault, DefaultGemmKLoop, LoopOver_MNK, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwd1x1P0,   DefaultGemmKLoop, LoopOver_MNK, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwdDefault, GemmKLoopOverC  , LoopOver_MKN, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwd1x1P0,   GemmKLoopOverC  , LoopOver_MKN, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwdDefault, DefaultGemmKLoop, LoopOver_MKN, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>, \
    DeviceConvNDFwdAvx2_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<float , float , float, a_elem_op, b_elem_op, c_elem_op, ConvFwd1x1P0,   DefaultGemmKLoop, LoopOver_MKN, 2, m_per_block, n_per_block, k_per_block, m_per_thread, n_per_thread, a_local_buf, b_local_buf, c_local_buf>

// clang-format on
using device_conv2d_fwd_avx2_nhwc_kyxc_nhwk_f32_instances = std::tuple<
    // clang-format off
    DEVICE_CONV2D_FWD_AVX2_NHWC_KYXC_NHWK_F32(PT, PT, PT,  256, 120,  64,  4, 24, true, true, false),
    DEVICE_CONV2D_FWD_AVX2_NHWC_KYXC_NHWK_F32(PT, PT, PT,  512, 144, 128,  4, 24, true, true, false),
    DEVICE_CONV2D_FWD_AVX2_NHWC_KYXC_NHWK_F32(PT, PT, PT,  512, 240, 128,  4, 24, true, true, false),
    // DEVICE_CONV2D_FWD_AVX2_NHWC_KYXC_NHWK_F32(PT, PT, PT,  768, 192, 128,  4, 24, true, true, false),
    DEVICE_CONV2D_FWD_AVX2_NHWC_KYXC_NHWK_F32(PT, PT, PT,  768, 288, 128,  4, 24, true, true, false)>;
// clang-format on

void add_device_conv2d_fwd_avx2_nhwc_kyxc_nhwk(std::vector<DeviceConvFwdPtr<PT, PT, PT>>& instances)
{
    ck::tensor_operation::device::add_device_operation_instances(
        instances, device_conv2d_fwd_avx2_nhwc_kyxc_nhwk_f32_instances{});
}

} // namespace device_conv2d_fwd_avx2_instance
} // namespace device
} // namespace cpu
} // namespace tensor_operation
} // namespace ck
