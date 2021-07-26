#include <unistd.h>
#include "device.hpp"
#include "host_tensor.hpp"
#include "driver_static_convolution_add_forward_implicit_gemm_v5r1_nchw_kcyx_nkhw_outpad.hpp"

template <typename TInWei,
          ck::index_t InWeiVectorSize,
          ck::index_t OutVectorSize,
          ck::index_t activ_type,
          typename TAcc,
          typename TOut,
          typename InLengths,
          typename WeiLengths,
          typename AddLengths,
          typename OutLengths,
          typename ConvStrides,
          typename ConvDilations,
          typename InLeftPads,
          typename InRightPads>
void device_static_convolution_add_forward_implicit_gemm_v5r1_nchw_kcyx_nkhw(
    const InLengths& in_n_c_hi_wi_lengths,
    const WeiLengths& wei_k_c_y_x_lengths,
    const AddLengths& add_n_k_hox2_wox2_lengths,
    const OutLengths& out_n_k_ho_wo_lengths,
    const ConvStrides& conv_strides,
    const ConvDilations& conv_dilations,
    const InLeftPads& in_left_pads,
    const InRightPads& in_right_pads,
    const Tensor<TInWei>& in_n_c_hi_wi,
    const Tensor<TInWei>& wei_k_c_y_x,
    const Tensor<TOut>& add_n_k_hox2_wox2,
    Tensor<TOut>& out_n_k_hox2_wox2,
    ck::index_t nrepeat)
{
    using namespace ck;

    std::cout << __func__ << std::endl;

    constexpr auto I0 = Number<0>{};
    constexpr auto I1 = Number<1>{};
    constexpr auto I2 = Number<2>{};
    constexpr auto I3 = Number<3>{};

    const auto N = out_n_k_ho_wo_lengths[I0];
    const auto K = out_n_k_ho_wo_lengths[I1];
    const auto C = wei_k_c_y_x_lengths[I1];

    const auto Hi = in_n_c_hi_wi_lengths[I2];
    const auto Wi = in_n_c_hi_wi_lengths[I3];

    const auto Ho = out_n_k_ho_wo_lengths[I2];
    const auto Wo = out_n_k_ho_wo_lengths[I3];

    const auto Hox2 = Ho * 2;
    const auto Wox2 = Wo * 2;

    const auto Y = wei_k_c_y_x_lengths[I2];
    const auto X = wei_k_c_y_x_lengths[I3];

    const auto C0 = C / Number<InWeiVectorSize>{};
    const auto C1 = Number<InWeiVectorSize>{};

    const auto K0 = K / Number<OutVectorSize>{};
    const auto K1 = Number<OutVectorSize>{};

    Tensor<TInWei> in_n_c0_hi_wi_c1(
        HostTensorDescriptor(std::initializer_list<index_t>{N, C0, Hi, Wi, C1}));
    Tensor<TInWei> wei_k_c0_y_x_c1(
        HostTensorDescriptor(std::initializer_list<index_t>{K, C0, Y, X, C1}));
    Tensor<TOut> out_n_k0_hox2_wox2_k1(
        HostTensorDescriptor(std::initializer_list<index_t>{N, K0, Hox2, Wox2, K1}));
    Tensor<TOut> add_n_k0_hox2_wox2_k1(
        HostTensorDescriptor(std::initializer_list<index_t>{N, K0, Hox2, Wox2, K1}));

    auto f_nchw2nc0hwc1 = [&](auto n, auto hi, auto wi, auto c) {
        in_n_c0_hi_wi_c1(n, c / InWeiVectorSize, hi, wi, c % InWeiVectorSize) =
            in_n_c_hi_wi(n, c, hi, wi);
    };

    auto f_kcyx2kc0yxc1 = [&](auto k, auto y, auto x, auto c) {
        wei_k_c0_y_x_c1(k, c / InWeiVectorSize, y, x, c % InWeiVectorSize) =
            wei_k_c_y_x(k, c, y, x);
    };

    auto f_nchx2wx2_to_nc0hx2wx2c1 = [&](auto n, auto ho, auto wo, auto c) {
        add_n_k0_hox2_wox2_k1(n, c / InWeiVectorSize, ho, wo, c % InWeiVectorSize) =
            add_n_k_hox2_wox2(n, c, ho, wo);
    };

    make_ParallelTensorFunctor(f_nchw2nc0hwc1, N, Hi, Wi, C)();
    make_ParallelTensorFunctor(f_kcyx2kc0yxc1, K, Y, X, C)();
    make_ParallelTensorFunctor(f_nchx2wx2_to_nc0hx2wx2c1, N, Hox2, Wox2, K)();

    DeviceMem in_n_c0_hi_wi_c1_device_buf(sizeof(TInWei) *
                                          in_n_c0_hi_wi_c1.mDesc.GetElementSpace());
    DeviceMem wei_k_c0_y_x_c1_device_buf(sizeof(TInWei) * wei_k_c0_y_x_c1.mDesc.GetElementSpace());
    DeviceMem add_n_k0_hox2_wox2_k1_device_buf(sizeof(TOut) *
                                               add_n_k0_hox2_wox2_k1.mDesc.GetElementSpace());
    DeviceMem out_n_k0_hox2_wox2_k1_device_buf(sizeof(TOut) *
                                               out_n_k0_hox2_wox2_k1.mDesc.GetElementSpace());

    in_n_c0_hi_wi_c1_device_buf.ToDevice(in_n_c0_hi_wi_c1.mData.data());
    wei_k_c0_y_x_c1_device_buf.ToDevice(wei_k_c0_y_x_c1.mData.data());

    const auto in_n_c0_hi_wi_desc =
        make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(N, C0, Hi, Wi));
    const auto wei_k_c0_y_x_desc =
        make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(K, C0, Y, X));
    const auto out_n_k0_ho_wo_k1_desc =
        make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(N, K0, Ho, Wo, K1));
    const auto add_n_k0_hox2_wox2_k1_desc =
        make_dynamic_naive_tensor_descriptor_packed_v2(make_tuple(N, K0, Hox2, Wox2, K1));

    // cdata = 64, BlockSize = 64, 16x8x32x4
    constexpr index_t BlockSize = 64;

    constexpr index_t KPerBlock  = K;
    constexpr index_t HoPerBlock = 8;
    constexpr index_t WoPerBlock = 32;
    constexpr index_t EPerBlock  = C0;

    constexpr index_t KPerThread  = KPerBlock;
    constexpr index_t HoPerThread = 2;
    constexpr index_t WoPerThread = 2;
    constexpr index_t EPerThread  = EPerBlock;

    using ABlockTransferThreadSliceLengths_E_K   = Sequence<Y * X, 1>;
    using ABlockTransferThreadClusterLengths_E_K = Sequence<EPerBlock, KPerBlock>;

    constexpr index_t ABlockTransferSrcScalarPerVector_E = 1;
    constexpr index_t ABlockTransferDstScalarPerVector_K = 1;

    constexpr index_t BThreadTransferSrcScalarPerVector_W = 1;

    constexpr index_t CThreadTransferDstScalarPerVector_W = K1;

    static_assert(KPerThread % CThreadTransferDstScalarPerVector_W == 0, "");

    constexpr auto conv_driver =
        DriverStaticConvolutionAddForwardImplicitGemm_v5r1_nchw_kcyx_nkhw_outpad<
            BlockSize,
            typename vector_type<TInWei, InWeiVectorSize>::type,
            TAcc,
            TOut,
            KPerBlock,
            HoPerBlock,
            WoPerBlock,
            EPerBlock,
            KPerThread,
            HoPerThread,
            WoPerThread,
            EPerThread,
            ABlockTransferThreadSliceLengths_E_K,
            ABlockTransferThreadClusterLengths_E_K,
            ABlockTransferSrcScalarPerVector_E,
            ABlockTransferDstScalarPerVector_K,
            BThreadTransferSrcScalarPerVector_W,
            CThreadTransferDstScalarPerVector_W>{};

    conv_driver.Run(wei_k_c0_y_x_desc,
                    in_n_c0_hi_wi_desc,
                    add_n_k0_hox2_wox2_k1_desc,
                    out_n_k0_ho_wo_k1_desc,
                    conv_strides,
                    conv_dilations,
                    in_left_pads,
                    in_right_pads,
                    Number<activ_type>{},
                    static_cast<typename vector_type<TInWei, InWeiVectorSize>::type*>(
                        wei_k_c0_y_x_c1_device_buf.GetDeviceBuffer()),
                    static_cast<typename vector_type<TInWei, InWeiVectorSize>::type*>(
                        in_n_c0_hi_wi_c1_device_buf.GetDeviceBuffer()),
                    static_cast<TOut*>(add_n_k0_hox2_wox2_k1_device_buf.GetDeviceBuffer()),
                    static_cast<TOut*>(out_n_k0_hox2_wox2_k1_device_buf.GetDeviceBuffer()));

    out_n_k0_hox2_wox2_k1_device_buf.FromDevice(out_n_k0_hox2_wox2_k1.mData.data());

    auto f_nk0hwk1_to_nkhw = [&](auto n, auto k, auto ho, auto wo) {
        out_n_k_hox2_wox2(n, k, ho, wo) =
            out_n_k0_hox2_wox2_k1(n, k / InWeiVectorSize, ho, wo, k % InWeiVectorSize);
    };

    make_ParallelTensorFunctor(f_nk0hwk1_to_nkhw, N, K, Hox2, Wox2)();
}
