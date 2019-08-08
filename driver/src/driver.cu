#include <iostream>
#include <numeric>
#include <initializer_list>
#include <cstdlib>
#include <stdlib.h>
#include "config.hpp"
#include "ConstantTensorDescriptor.hpp"
#include "device.hpp"
#include "conv_common.hpp"
#include "host_conv.hpp"
#include "device_convolution_direct_v2_nchw_kcyx_nkhw.hpp"
//#include "device_convolution_implicit_gemm_v1_chwn_cyxk_khwn.hpp"
//#include "device_convolution_implicit_gemm_v1_nchw_cyxk_nkhw.hpp"
//#include "device_convolution_implicit_gemm_v2_chwn_cyxk_khwn.hpp"
//#include "device_convolution_implicit_gemm_v3_nchw_cyxk_nkhw.hpp"
#include "device_convolution_implicit_gemm_v4r1_nchw_kcyx_nkhw.hpp"
//#include "device_convolution_implicit_gemm_v4r2_nchw_kcyx_nkhw.hpp"
//#include "device_convolution_implicit_gemm_v4r3_nchw_kcyx_nkhw.hpp"
#include "device_convolution_implicit_gemm_v4r4_nchw_kcyx_nkhw.hpp"

struct GeneratorTensor_1
{
    template <class... Is>
    double operator()(Is... is)
    {
        return 1;
    }
};

struct GeneratorTensor_2
{
    int min_value = 0;
    int max_value = 1;

    template <class... Is>
    double operator()(Is...)
    {
        return (std::rand() % (max_value - min_value)) + min_value;
    }
};

struct GeneratorTensor_3
{
    template <class... Is>
    double operator()(Is... is)
    {
        std::array<index_t, sizeof...(Is)> dims = {{static_cast<index_t>(is)...}};

        auto f_acc = [](auto a, auto b) { return 100 * a + b; };

        return std::accumulate(dims.begin(), dims.end(), index_t(0), f_acc);
    }
};

struct GeneratorTensor_Checkboard
{
    template <class... Ts>
    double operator()(Ts... Xs) const
    {
        std::array<index_t, sizeof...(Ts)> dims = {{Xs...}};
        return std::accumulate(dims.begin(),
                               dims.end(),
                               true,
                               [](bool init, index_t x) -> int { return init != (x % 2); })
                   ? 1
                   : -1;
    }
};

int main(int argc, char* argv[])
{
    using namespace ck;

#if 1
    constexpr index_t N  = 128;
    constexpr index_t C  = 256;
    constexpr index_t HI = 35;
    constexpr index_t WI = 35;
    constexpr index_t K  = 384;
    constexpr index_t Y  = 3;
    constexpr index_t X  = 3;

    using ConvStrides   = Sequence<2, 2>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 3x3, 34x34
    constexpr index_t N  = 64;
    constexpr index_t C  = 256;
    constexpr index_t HI = 34;
    constexpr index_t WI = 34;
    constexpr index_t K  = 128;
    constexpr index_t Y  = 3;
    constexpr index_t X  = 3;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 8x8 image
    // cudnn@V100 68%, ck@V100 72%, ck@P100 52%, ck@VII 42%
    constexpr index_t N  = 64;
    constexpr index_t C  = 1536;
    constexpr index_t HI = 8;
    constexpr index_t WI = 8;
    constexpr index_t K  = 256;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 8x8 image
    // cudnn@V100 77%, ck@V100 76%, ck@P100 79%, ck@VII 51%
    constexpr index_t N  = 128;
    constexpr index_t C  = 2048;
    constexpr index_t HI = 8;
    constexpr index_t WI = 8;
    constexpr index_t K  = 384;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 7x7 image
    // cudnn@V100 82%, ck@V100 76%, ck@P100 67%, ck@VII 64%
    constexpr index_t N  = 128;
    constexpr index_t C  = 832;
    constexpr index_t HI = 7;
    constexpr index_t WI = 7;
    constexpr index_t K  = 384;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 8x8 image
    // cudnn@V100 83%, ck@V100 75%, ck@P100 78%, ck@VII 65%
    constexpr index_t N  = 128;
    constexpr index_t C  = 1280;
    constexpr index_t HI = 8;
    constexpr index_t WI = 8;
    constexpr index_t K  = 384;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 14x14 image
    // cudnn@V100 62%, ck@V100 68%, ck@P100 70%, ck@VII 50%
    constexpr index_t N  = 128;
    constexpr index_t C  = 512;
    constexpr index_t HI = 14;
    constexpr index_t WI = 14;
    constexpr index_t K  = 128;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 8x8 image
    // cudnn@V100 74%, ck@V100 57%, ck@P100 78%, ck@VII 61%
    constexpr index_t N  = 64;
    constexpr index_t C  = 1536;
    constexpr index_t HI = 8;
    constexpr index_t WI = 8;
    constexpr index_t K  = 384;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 28x28 image
    // cudnn@V100 86%, ck@V100 84%, ck@P100 80%, ck@VII 69%
    constexpr index_t N  = 128;
    constexpr index_t C  = 256;
    constexpr index_t HI = 28;
    constexpr index_t WI = 28;
    constexpr index_t K  = 128;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 7x7 image
    // cudnn@V100 71%, ck@V100 55%, ck@P100 70%, ck@VII 62%
    constexpr index_t N  = 128;
    constexpr index_t C  = 832;
    constexpr index_t HI = 7;
    constexpr index_t WI = 7;
    constexpr index_t K  = 256;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 1
    // 3x3 filter, 2x2 stride, 35x35 input, 17x17 output
    // cudnn@V100 90%, ck@V100 93%, ck@P100 83%, ck@VII 81%
    constexpr index_t N  = 128;
    constexpr index_t C  = 288;
    constexpr index_t HI = 35;
    constexpr index_t WI = 35;
    constexpr index_t K  = 384;
    constexpr index_t Y  = 3;
    constexpr index_t X  = 3;

    using ConvStrides   = Sequence<2, 2>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 1
    // 1x1 filter, 17x17 input
    // cudnn@V100 81%, ck@V100 76%, ck@P100 70%, ck@VII 76%
    constexpr index_t N  = 128;
    constexpr index_t C  = 768;
    constexpr index_t HI = 17;
    constexpr index_t WI = 17;
    constexpr index_t K  = 128;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 14x14 image
    // cudnn@V100 73%, ck@V100 71%, ck@P100 70%, ck@VII 64%
    constexpr index_t N  = 128;
    constexpr index_t C  = 528;
    constexpr index_t HI = 14;
    constexpr index_t WI = 14;
    constexpr index_t K  = 128;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 14x14 image
    // cudnn@V100 73%, ck@V100 72%, ck@P100 79%, ck@VII 75%
    constexpr index_t N  = 128;
    constexpr index_t C  = 528;
    constexpr index_t HI = 14;
    constexpr index_t WI = 14;
    constexpr index_t K  = 256;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#elif 0
    // 1x1 filter, 7x7 image
    // cudnn@V100 49%, ck@V100 50%, ck@P100 61%, ck@VII 52%
    constexpr index_t N  = 128;
    constexpr index_t C  = 832;
    constexpr index_t HI = 7;
    constexpr index_t WI = 7;
    constexpr index_t K  = 128;
    constexpr index_t Y  = 1;
    constexpr index_t X  = 1;

    using ConvStrides   = Sequence<1, 1>;
    using ConvDilations = Sequence<1, 1>;

    constexpr index_t HPad = 0;
    constexpr index_t WPad = 0;
#endif

    auto lower_pads = Sequence<HPad, WPad>{};
    auto upper_pads = Sequence<HPad, WPad>{};

    auto in_nchw_desc  = make_ConstantTensorDescriptor_packed(Sequence<N, C, HI, WI>{});
    auto wei_kcyx_desc = make_ConstantTensorDescriptor_packed(Sequence<K, C, Y, X>{});
    auto out_nkhw_desc = get_convolution_with_padding_output_default_4d_tensor_descriptor(
        in_nchw_desc, wei_kcyx_desc, ConvStrides{}, ConvDilations{}, lower_pads, upper_pads);

    ostream_ConstantTensorDescriptor(in_nchw_desc, std::cout << "in_nchw_desc: ");
    ostream_ConstantTensorDescriptor(wei_kcyx_desc, std::cout << "wei_kcyx_desc: ");
    ostream_ConstantTensorDescriptor(out_nkhw_desc, std::cout << "out_nkhw_desc: ");

    // for backward weight
    auto in_nchw_wrw_desc  = in_nchw_desc.ReorderGivenNew2Old(Sequence<1, 0, 2, 3>{});
    auto wei_kcyx_wrw_desc = out_nkhw_desc.ReorderGivenNew2Old(Sequence<1, 0, 2, 3>{});
    auto out_nkhw_wrw_desc = wei_kcyx_desc.ReorderGivenNew2Old(Sequence<1, 0, 2, 3>{});

    ostream_ConstantTensorDescriptor(in_nchw_wrw_desc, std::cout << "in_nchw_wrw_desc: ");
    ostream_ConstantTensorDescriptor(wei_kcyx_wrw_desc, std::cout << "wei_kcyx_wrw_desc: ");
    ostream_ConstantTensorDescriptor(out_nkhw_wrw_desc, std::cout << "out_nkhw_wrw_desc: ");

    using in_data_t  = float;
    using out_data_t = float;
    Tensor<in_data_t> in_nchw_wrw(make_TensorDescriptor(in_nchw_wrw_desc));
    Tensor<in_data_t> wei_kcyx_wrw(make_TensorDescriptor(wei_kcyx_wrw_desc));
    Tensor<out_data_t> out_nkhw_wrw_host(make_TensorDescriptor(out_nkhw_wrw_desc));
    Tensor<out_data_t> out_nkhw_wrw_device(make_TensorDescriptor(out_nkhw_wrw_desc));

    std::size_t num_thread = std::thread::hardware_concurrency();

    if(argc != 3)
    {
        printf("arg1: do_verification, arg2: nrepeat\n");
        exit(1);
    }

    bool do_verification = atoi(argv[1]);
    index_t nrepeat      = atoi(argv[2]);

    if(do_verification)
    {
#if 0
        in_nchw_wrw.GenerateTensorValue(GeneratorTensor_1{}, num_thread);
        wei_kcyx_wrw.GenerateTensorValue(GeneratorTensor_1{}, num_thread);
#elif 0
        in_nchw.GenerateTensorValue(GeneratorTensor_1{}, num_thread);
        wei_kcyx.GenerateTensorValue(GeneratorTensor_2{-5, 5}, num_thread);
#elif 0
        in_nchw.GenerateTensorValue(GeneratorTensor_3{}, num_thread);
        wei_kcyx.GenerateTensorValue(GeneratorTensor_1{}, num_thread);
#elif 1
        in_nchw_wrw.GenerateTensorValue(GeneratorTensor_2{-5, 5}, num_thread);
        wei_kcyx_wrw.GenerateTensorValue(GeneratorTensor_2{-5, 5}, num_thread);
#elif 0
        in_nchw.GenerateTensorValue(GeneratorTensor_2{1, 5}, num_thread);

        auto gen_wei = [](auto... is) {
            return GeneratorTensor_2{1, 5}(is...) * GeneratorTensor_Checkboard{}(is...);
        };
        wei_kcyx.GenerateTensorValue(gen_wei, num_thread);
#endif
    }

#if 0
    device_convolution_direct_v2_nchw_kcyx_nkhw
        (in_nchw_desc, in_nchw, wei_kcyx_desc, wei_kcyx, out_nkhw_desc, out_nkhw_device, nrepeat);
#elif 0
    device_convolution_implicit_gemm_v1_chwn_cyxk_khwn(
        in_nchw_desc, in_nchw, wei_kcyx_desc, wei_kcyx, out_nkhw_desc, out_nkhw_device, nrepeat);
#elif 0
    device_convolution_implicit_gemm_v1_nchw_cyxk_nkhw(
        in_nchw_desc, in_nchw, wei_kcyx_desc, wei_kcyx, out_nkhw_desc, out_nkhw_device, nrepeat);
#elif 0
    device_convolution_implicit_gemm_v2_chwn_cyxk_khwn(
        in_nchw_desc, in_nchw, wei_kcyx_desc, wei_kcyx, out_nkhw_desc, out_nkhw_device, nrepeat);
#elif 0
    device_convolution_implicit_gemm_v3_nchw_cyxk_nkhw(
        (in_nchw_desc, in_nchw, wei_kcyx_desc, wei_kcyx, out_nkhw_desc, out_nkhw_device, nrepeat);
#elif 1
    device_convolution_implicit_gemm_v4r1_nchw_kcyx_nkhw(
        in_nchw_wrw_desc,
        in_nchw_wrw,
        wei_kcyx_wrw_desc,
        wei_kcyx_wrw,
        out_nkhw_wrw_desc,
        out_nkhw_wrw_device,
        ConvDilations{}, // exchange dilation and strides
        ConvStrides{},
        nrepeat);
#elif 0
    device_convolution_implicit_gemm_v4r2_nchw_kcyx_nkhw(in_nchw_desc,
                                                         in_nchw,
                                                         wei_kcyx_desc,
                                                         wei_kcyx,
                                                         out_nkhw_desc,
                                                         out_nkhw_device,
                                                         ConvStrides{},
                                                         ConvDilations{},
                                                         nrepeat);
#elif 0
    device_convolution_implicit_gemm_v4r3_nchw_kcyx_nkhw(in_nchw_desc,
                                                         in_nchw,
                                                         wei_kcyx_desc,
                                                         wei_kcyx,
                                                         out_nkhw_desc,
                                                         out_nkhw_device,
                                                         ConvStrides{},
                                                         ConvDilations{},
                                                         nrepeat);
#elif 1
    device_convolution_implicit_gemm_v4r4_nchw_kcyx_nkhw(in_nchw_desc,
                                                         in_nchw,
                                                         wei_kcyx_desc,
                                                         wei_kcyx,
                                                         out_nkhw_desc,
                                                         out_nkhw_device,
                                                         ConvStrides{},
                                                         ConvDilations{},
                                                         nrepeat);
#elif 0
    device_implicit_gemm_convolution_1_chwn_cyxk_khwn_padded(in_nchw_desc,
                                                             in_nchw,
                                                             wei_kcyx_desc,
                                                             wei_kcyx,
                                                             out_nkhw_desc,
                                                             out_nkhw_device,
                                                             lower_pads,
                                                             upper_pads,
                                                             nrepeat);
#endif

    if(do_verification)
    {
#if 0
        if(Y == 3 && X == 3 && ConvStrides{}[0] == 1 && ConvStrides{}[1] == 1 &&
           ConvDilations{}[0] == 1 && ConvDilations{}[1] == 1)
        {
            host_winograd_3x3_convolution(in_nchw, wei_kcyx, out_nkhw_host, lower_pads, upper_pads);
        }
        else
#endif
        {
            host_direct_convolution(in_nchw_wrw,
                                    wei_kcyx_wrw,
                                    out_nkhw_wrw_host,
                                    ConvDilations{},
                                    ConvStrides{},
                                    lower_pads,
                                    upper_pads);
        }
        check_error(out_nkhw_wrw_host, out_nkhw_wrw_device);

#if 0
        LogRange(std::cout << "in_nchw_wrw : ", in_nchw_wrw.mData, ",") << std::endl;
        LogRange(std::cout << "wei_kcyx_wrw: ", wei_kcyx_wrw.mData, ",") << std::endl;
        LogRange(std::cout << "out_nkhw_wrw_host  : ", out_nkhw_wrw_host.mData, ",") << std::endl;
        LogRange(std::cout << "out_nkhw_wrw_device: ", out_nkhw_wrw_device.mData, ",") << std::endl;
#endif
    }
}
