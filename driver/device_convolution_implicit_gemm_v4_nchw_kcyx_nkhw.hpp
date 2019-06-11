#pragma once
#include <unistd.h>
#include "device.hpp"
#include "gridwise_convolution_wrapper.hip.hpp"
#include "gridwise_convolution_implicit_gemm_v4_nchw_kcyx_nkhw.hip.hpp"
#include "gridwise_convolution_implicit_gemm_v4_lds_double_buffer_nchw_kcyx_nkhw.hip.hpp"

template <class T,
          class InDesc,
          class WeiDesc,
          class OutDesc,
          class Strides,
          class Dilations,
          index_t Direction>
void device_convolution_implicit_gemm_v4_nchw_kcyx_nkhw(InDesc,
                                                        const Tensor<T>& in_nchw,
                                                        WeiDesc,
                                                        const Tensor<T>& wei_kcyx,
                                                        OutDesc,
                                                        Strides,
                                                        Dilations,
                                                        Number<Direction>,
                                                        Tensor<T>& out_nkhw,
                                                        index_t nrepeat)
{
    constexpr auto I0 = Number<0>{};
    constexpr auto I1 = Number<1>{};
    constexpr auto I2 = Number<2>{};
    constexpr auto I3 = Number<3>{};

    constexpr auto in_nchw_desc  = InDesc{};
    constexpr auto wei_kcyx_desc = WeiDesc{};
    constexpr auto out_nkhw_desc = OutDesc{};

    constexpr index_t Hi = in_nchw_desc.GetLength(I2);
    constexpr index_t Wi = in_nchw_desc.GetLength(I3);

    constexpr index_t N  = out_nkhw_desc.GetLength(I0);
    constexpr index_t Ho = out_nkhw_desc.GetLength(I2);
    constexpr index_t Wo = out_nkhw_desc.GetLength(I3);

    constexpr index_t K = wei_kcyx_desc.GetLength(I0);
    constexpr index_t C = wei_kcyx_desc.GetLength(I1);
    constexpr index_t Y = wei_kcyx_desc.GetLength(I2);
    constexpr index_t X = wei_kcyx_desc.GetLength(I3);

    std::size_t data_sz = sizeof(T);
    DeviceMem in_nchw_device_buf(data_sz * in_nchw.mDesc.GetElementSpace());
    DeviceMem wei_kcyx_device_buf(data_sz * wei_kcyx.mDesc.GetElementSpace());
    DeviceMem out_nkhw_device_buf(data_sz * out_nkhw.mDesc.GetElementSpace());

    in_nchw_device_buf.ToDevice(in_nchw.mData.data());
    wei_kcyx_device_buf.ToDevice(wei_kcyx.mData.data());
    out_nkhw_device_buf.ToDevice(out_nkhw.mData.data());

    constexpr index_t N1 = 2;
    constexpr index_t N2 = 4;

    constexpr index_t B = N * mod_conv::integer_divide_ceil(Ho, Strides::Get(I0)) *
                          mod_conv::integer_divide_ceil(Wo, Strides::Get(I1)) / (N1 * N2);

#if 1
    constexpr index_t BlockSize = 256;

    constexpr index_t BPerBlock = 16;
    constexpr index_t KPerBlock = 128;
    constexpr index_t CPerBlock = 8;

    constexpr index_t GemmMPerThreadSubC = 4;
    constexpr index_t GemmNPerThreadSubC = 4;
    constexpr index_t GemmMLevel0Cluster = 4;
    constexpr index_t GemmNLevel0Cluster = 4;
    constexpr index_t GemmMLevel1Cluster = 4;
    constexpr index_t GemmNLevel1Cluster = 4;
    constexpr index_t GemmKPerThreadLoop = 1;
    constexpr index_t GemmDataPerReadA   = 4;
    constexpr index_t GemmDataPerReadB   = 4;

    using InBlockCopySubLengths_E_N1_B_N2      = Sequence<1, 1, 1, 4>;
    using InBlockCopyClusterLengths_E_N1_B_N2  = Sequence<8, 2, 16, 1>;
    using InBlockCopyThreadClusterArrangeOrder = Sequence<0, 1, 3, 2>; // [E, N1, N2, B]
    using InBlockCopySrcAccessOrder            = Sequence<0, 1, 3, 2>; // [E, N1, N2, B]
    using InBlockCopyDstAccessOrder            = Sequence<0, 1, 2, 3>; // [E, N1, B, N2]

    constexpr index_t InBlockCopySrcDataPerRead_B   = 1;
    constexpr index_t InBlockCopyDstDataPerWrite_N2 = 4;

    using WeiBlockCopySubLengths_E_K            = Sequence<1, 4>;
    using WeiBlockCopyClusterLengths_E_K        = Sequence<8, 32>;
    using WeiBlockCopyThreadClusterArrangeOrder = Sequence<1, 0>; // [K, E]
    using WeiBlockCopySrcAccessOrder            = Sequence<1, 0>; // [K, E]
    using WeiBlockCopyDstAccessOrder            = Sequence<0, 1>; // [E, K]

    constexpr index_t WeiBlockCopySrcDataPerRead_E  = 1;
    constexpr index_t WeiBlockCopyDstDataPerWrite_K = 4;
#endif

    constexpr index_t GridSize =
        ((B + BPerBlock - 1) / BPerBlock) * ((K + KPerBlock - 1) / KPerBlock);

    printf("%s: BlockSize %u, GridSize %u \n", __func__, BlockSize, GridSize);

    for(index_t i = 0; i < nrepeat; ++i)
    {
        constexpr auto gridwise_conv =
#if 0
            GridwiseConvolutionImplicitGemm_v4_nchw_kcyx_nkhw
#else
            GridwiseConvolutionImplicitGemm_v4_lds_double_buffer_nchw_kcyx_nkhw
#endif
            <GridSize,
             BlockSize,
             Strides,
             Dilations,
             T,
             decltype(in_nchw_desc),
             decltype(wei_kcyx_desc),
             decltype(out_nkhw_desc),
             BPerBlock,
             KPerBlock,
             CPerBlock,
             N1,
             N2,
             GemmMPerThreadSubC,
             GemmNPerThreadSubC,
             GemmMLevel0Cluster,
             GemmNLevel0Cluster,
             GemmMLevel1Cluster,
             GemmNLevel1Cluster,
             GemmKPerThreadLoop,
             GemmDataPerReadA,
             GemmDataPerReadB,
             InBlockCopySubLengths_E_N1_B_N2,
             InBlockCopyClusterLengths_E_N1_B_N2,
             InBlockCopyThreadClusterArrangeOrder,
             InBlockCopySrcAccessOrder,
             InBlockCopyDstAccessOrder,
             InBlockCopySrcDataPerRead_B,
             InBlockCopyDstDataPerWrite_N2,
             WeiBlockCopySubLengths_E_K,
             WeiBlockCopyClusterLengths_E_K,
             WeiBlockCopyThreadClusterArrangeOrder,
             WeiBlockCopySrcAccessOrder,
             WeiBlockCopyDstAccessOrder,
             WeiBlockCopySrcDataPerRead_E,
             WeiBlockCopyDstDataPerWrite_K>{};

        float time = launch_kernel(run_gridwise_convolution<decltype(gridwise_conv), T>,
                                   dim3(GridSize),
                                   dim3(BlockSize),
                                   0,
                                   static_cast<T*>(in_nchw_device_buf.GetDeviceBuffer()),
                                   static_cast<T*>(wei_kcyx_device_buf.GetDeviceBuffer()),
                                   static_cast<T*>(out_nkhw_device_buf.GetDeviceBuffer()));

        printf("Elapsed time : %f ms, %f TFlop/s\n",
               time,
               (float)calculate_convolution_flops(InDesc{}, WeiDesc{}, OutDesc{}) /
                   (std::size_t(1000) * 1000 * 1000) / time);
        usleep(std::min(time * 1000, float(10000)));
    }

    out_nkhw_device_buf.FromDevice(out_nkhw.mData.data());
}
