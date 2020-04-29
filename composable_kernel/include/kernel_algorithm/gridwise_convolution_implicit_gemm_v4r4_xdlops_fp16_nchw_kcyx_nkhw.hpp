#ifndef CK_GRIDWISE_CONVOLUTION_IMPLICIT_GEMM_V4R4_XDLOPS_FP16_NCHW_KCYX_NKHW_HPP
#define CK_GRIDWISE_CONVOLUTION_IMPLICIT_GEMM_V4R4_XDLOPS_FP16_NCHW_KCYX_NKHW_HPP

#include "common_header.hpp"
#include "tensor_descriptor.hpp"
#include "tensor_descriptor_helper.hpp"
#include "gridwise_gemm_xdlops_fp16_bfp16.hpp"

namespace ck {

template <index_t GemmKPACK>
struct make_vectorized_WeiDesc_Xdlops
{
    template <typename WeiDesc>
    __device__ constexpr auto get(WeiDesc&)
    {
        constexpr auto I0 = Number<0>{};
        constexpr auto I1 = Number<1>{};
        constexpr auto I2 = Number<2>{};
        constexpr auto I3 = Number<3>{};

        constexpr auto wei_k_c_y_x_global_desc = WeiDesc{};

        constexpr index_t K = wei_k_c_y_x_global_desc.GetLength(I0);
        constexpr index_t C = wei_k_c_y_x_global_desc.GetLength(I1);
        constexpr index_t Y = wei_k_c_y_x_global_desc.GetLength(I2);
        constexpr index_t X = wei_k_c_y_x_global_desc.GetLength(I3);

        /*     kpack comes from c*y*x  */
        static_assert((C * Y * X) % GemmKPACK == 0,
                      "C needs to be multiple of vectorized GemmKPACK");
        constexpr index_t GemmK = (C * Y * X) / GemmKPACK;

        constexpr auto wei_gemmm_gemmk_global_desc =
            transform_tensor_descriptor(unfold_tensor_descriptor(wei_k_c_y_x_global_desc, I1, I3),
                                        make_tuple(PassThrough<K>{}, PassThrough<C * Y * X>{}),
                                        make_tuple(Sequence<0>{}, Sequence<1>{}),
                                        make_tuple(Sequence<0>{}, Sequence<1>{}));

        constexpr auto wei_gemmm_gemmk_gemmkpack_global_desc = transform_tensor_descriptor(
            wei_gemmm_gemmk_global_desc,
            make_tuple(PassThrough<K>{}, UnMerge<Sequence<GemmK, GemmKPACK>>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}),
            make_tuple(Sequence<0>{}, Sequence<1, 2>{}));

        constexpr auto wei_gemmk_gemmm_gemmkpack_global_desc = transform_tensor_descriptor(
            wei_gemmm_gemmk_gemmkpack_global_desc,
            make_tuple(PassThrough<GemmK>{}, PassThrough<K>{}, PassThrough<GemmKPACK>{}),
            make_tuple(Sequence<1>{}, Sequence<0>{}, Sequence<2>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}));

        return wei_gemmk_gemmm_gemmkpack_global_desc;
    }
};
// GemmM = K
// GemmN = N * Ho * Wo
// GemmK = C * Y * X
template <index_t GridSize,
          index_t BlockSize,
          class Float,
          class AccDataType,
          class InGlobalDesc,
          class WeiGlobalDesc,
          class OutGlobalDesc,
          class ConvStrides,
          class ConvDilations,
          class LeftPads,
          class RightPads,
          index_t GemmMPerBlock,
          index_t GemmNPerBlock,
          index_t GemmKPerBlock,
          index_t GemmKPACK,
          index_t GemmMPerWave,
          index_t GemmNPerWave,
          index_t GemmThreadGemmDataPerReadM,
          index_t GemmThreadGemmDataPerReadN,
          class GemmABlockCopyThreadSliceLengths_GemmK_GemmM_GemmKPACK,
          class GemmABlockCopyThreadClusterLengths_GemmK_GemmM_GemmKPACK,
          index_t GemmABlockCopySrcDataPerRead_GemmKPACK,
          index_t GemmABlockCopyDstDataPerWrite_GemmKPACK,
          class GemmBBlockCopyThreadSliceLengths_GemmK_GemmN_GemmKPACK,
          class GemmBBlockCopyThreadClusterLengths_GemmK_GemmN_GemmKPACK,
          index_t GemmBBlockCopySrcDataPerRead_GemmN,
          index_t GemmBBlockCopyDstDataPerWrite_GemmKPACK>
struct GridwiseConvolutionImplicitGemm_v4r4_xdlops_fwd_fp16_nchw_kcyx_nkhw
{
    __device__ void Run(const Float* const __restrict__ p_in_global,
                        const Float* const __restrict__ p_wei_global,
                        Float* const __restrict__ p_out_global) const
    {
        constexpr auto I0 = Number<0>{};
        constexpr auto I1 = Number<1>{};
        constexpr auto I2 = Number<2>{};
        constexpr auto I3 = Number<3>{};

        constexpr auto in_n_c_hi_wi_global_desc  = InGlobalDesc{};
        constexpr auto wei_k_c_y_x_global_desc   = WeiGlobalDesc{};
        constexpr auto out_n_k_ho_wo_global_desc = OutGlobalDesc{};

        constexpr index_t N  = in_n_c_hi_wi_global_desc.GetLength(I0);
        constexpr index_t C  = in_n_c_hi_wi_global_desc.GetLength(I1);
        constexpr index_t Hi = in_n_c_hi_wi_global_desc.GetLength(I2);
        constexpr index_t Wi = in_n_c_hi_wi_global_desc.GetLength(I3);

        constexpr index_t K  = out_n_k_ho_wo_global_desc.GetLength(I1);
        constexpr index_t Ho = out_n_k_ho_wo_global_desc.GetLength(I2);
        constexpr index_t Wo = out_n_k_ho_wo_global_desc.GetLength(I3);

        constexpr index_t Y = wei_k_c_y_x_global_desc.GetLength(I2);
        constexpr index_t X = wei_k_c_y_x_global_desc.GetLength(I3);

        constexpr index_t GemmM = K;
        constexpr index_t GemmK = (C * Y * X) / GemmKPACK;
        constexpr index_t GemmN = N * Ho * Wo;

        static_assert(GemmM % GemmMPerBlock == 0 && GemmN % GemmNPerBlock == 0 &&
                          GemmK % GemmKPerBlock == 0,
                      "wrong! cannot divide work evenly among block");

        // sanity-check for vectorized memory load
        constexpr index_t ConvStrideH = ConvStrides{}[0];
        constexpr index_t ConvStrideW = ConvStrides{}[1];

        constexpr index_t ConvDilationH = ConvDilations{}[0];
        constexpr index_t ConvDilationW = ConvDilations{}[1];

        static_assert((Wo == 1 || (ConvStrideW == 1 || GemmBBlockCopySrcDataPerRead_GemmN == 1)) &&
                          (X == 1 || ConvDilationW % GemmBBlockCopySrcDataPerRead_GemmN == 0),
                      "wrong! aligment requirement for vectorized global load of input tensor will "
                      "be violated");

        constexpr auto in_n_c_hip_wip_global_desc = transform_tensor_descriptor(
            in_n_c_hi_wi_global_desc,
            make_tuple(
                PassThrough<N>{}, PassThrough<C>{}, Pad<Sequence<Hi, Wi>, LeftPads, RightPads>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2, 3>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2, 3>{}));

        constexpr index_t Hip = in_n_c_hip_wip_global_desc.GetLengths()[2];
        constexpr index_t Wip = in_n_c_hip_wip_global_desc.GetLengths()[3];

        constexpr auto in_n_c_y_ho_x_wo_global_desc = transform_tensor_descriptor(
            in_n_c_hip_wip_global_desc,
            make_tuple(PassThrough<N>{},
                       PassThrough<C>{},
                       Embed<Hip, Sequence<Y, Ho>, Sequence<ConvDilationH, ConvStrideH, 0>>{},
                       Embed<Wip, Sequence<X, Wo>, Sequence<ConvDilationW, ConvStrideW, 0>>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2, 3>{}, Sequence<4, 5>{}));

        constexpr auto in_gemmk_gemmn_global_desc = transform_tensor_descriptor(
            in_n_c_y_ho_x_wo_global_desc,
            make_tuple(Merge<Sequence<C, Y, X>>{}, Merge<Sequence<N, Ho, Wo>>{}),
            make_tuple(Sequence<1, 2, 4>{}, Sequence<0, 3, 5>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}));

        constexpr auto in_gemmk_gemmkpack_gemmn_global_desc = transform_tensor_descriptor(
            in_gemmk_gemmn_global_desc,
            make_tuple(UnMerge<Sequence<GemmK, GemmKPACK>>{}, PassThrough<GemmN>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}),
            make_tuple(Sequence<0, 1>{}, Sequence<2>{}));

        constexpr auto in_gemmk_gemmn_gemmkpack_global_desc = transform_tensor_descriptor(
            in_gemmk_gemmkpack_gemmn_global_desc,
            make_tuple(PassThrough<GemmK>{}, PassThrough<GemmN>{}, PassThrough<GemmKPACK>{}),
            make_tuple(Sequence<0>{}, Sequence<2>{}, Sequence<1>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}));

        constexpr auto wei_gemmk_gemmm_gemmkpack_global_desc =
            make_vectorized_WeiDesc_Xdlops<GemmKPACK>{}.get(wei_k_c_y_x_global_desc);

        constexpr auto out_gemmm_gemmn_global_desc =
            transform_tensor_descriptor(out_n_k_ho_wo_global_desc,
                                        make_tuple(PassThrough<K>{}, Merge<Sequence<N, Ho, Wo>>{}),
                                        make_tuple(Sequence<1>{}, Sequence<0, 2, 3>{}),
                                        make_tuple(Sequence<0>{}, Sequence<1>{}));

        // GEMM
        constexpr auto gridwise_gemm = GridwiseGemmTransposedANormalBNormalCXdlopsFp16Bfp16_v1<
            GridSize,
            BlockSize,
            Float,
            AccDataType,
            Float,
            decltype(wei_gemmk_gemmm_gemmkpack_global_desc),
            decltype(in_gemmk_gemmn_gemmkpack_global_desc),
            decltype(out_gemmm_gemmn_global_desc),
            GemmMPerBlock,
            GemmNPerBlock,
            GemmKPerBlock,
            GemmMPerWave,
            GemmNPerWave,
            GemmThreadGemmDataPerReadM,
            GemmThreadGemmDataPerReadN,
            GemmABlockCopyThreadSliceLengths_GemmK_GemmM_GemmKPACK,
            GemmABlockCopyThreadClusterLengths_GemmK_GemmM_GemmKPACK,
            Sequence<1, 0, 2>,
            Sequence<1, 0, 2>,
            Sequence<0, 1, 2>,
            2,
            GemmABlockCopySrcDataPerRead_GemmKPACK,
            GemmABlockCopyDstDataPerWrite_GemmKPACK,
            GemmBBlockCopyThreadSliceLengths_GemmK_GemmN_GemmKPACK,
            GemmBBlockCopyThreadClusterLengths_GemmK_GemmN_GemmKPACK,
            Sequence<0, 2, 1>,
            Sequence<0, 2, 1>,
            Sequence<0, 1, 2>,
            1,
            GemmBBlockCopySrcDataPerRead_GemmN,
            GemmBBlockCopyDstDataPerWrite_GemmKPACK,
            InMemoryDataOperation::Set>{};

        gridwise_gemm.Run(p_wei_global, p_in_global, p_out_global);
    }
};

} // namespace ck
#endif
