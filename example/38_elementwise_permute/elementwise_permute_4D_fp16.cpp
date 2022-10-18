#include <iostream>
#include <cstdlib>

#include "ck/ck.hpp"
#include "ck/tensor_operation/gpu/element/binary_element_wise_operation.hpp"
#include "ck/tensor_operation/gpu/device/device_elementwise.hpp"

#include "ck/library/utility/check_err.hpp"
#include "ck/library/utility/device_memory.hpp"
#include "ck/library/utility/host_tensor.hpp"
#include "ck/library/utility/host_tensor_generator.hpp"

using F16 = ck::half_t;
using F32 = float;

using ADataType = F16;
using BDataType = F16;

using PassThrough = ck::tensor_operation::element_wise::PassThrough;
using DeviceElementwisePermuteInstance =
    ck::tensor_operation::device::DeviceElementwise<ck::Tuple<ADataType>,
                                                    ck::Tuple<BDataType>,
                                                    PassThrough,
                                                    4,
                                                    8,
                                                    ck::Sequence<8>,
                                                    ck::Sequence<1>>;

template <typename HostTensorA, typename HostTensorB, typename Functor>
void host_elementwise4D(HostTensorB& B_nhwc, const HostTensorA& A_nchw, Functor functor)
{
    for(std::size_t n = 0; n < A_nchw.mDesc.GetLengths()[0]; ++n)
        for(std::size_t c = 0; c < A_nchw.mDesc.GetLengths()[1]; ++c)
            for(std::size_t h = 0; h < A_nchw.mDesc.GetLengths()[2]; ++h)
                for(std::size_t w = 0; w < A_nchw.mDesc.GetLengths()[3]; ++w)
                {
                    auto a_val = A_nchw(n, c, h, w);
                    functor(B_nhwc(n, h, w, c), a_val);
                }
}

int main()
{
    bool do_verification = true;
    bool time_kernel     = false;

    std::vector<std::size_t> nchw = {4, 4, 8, 8};
    std::vector<std::size_t> nhwc = {4, 8, 8, 4};
    Tensor<ADataType> a(nchw);
    Tensor<BDataType> b(nhwc);

    a.GenerateTensorValue(GeneratorTensor_3<ADataType>{0.0, 1.0});

    DeviceMem a_device_buf(sizeof(ADataType) * a.mDesc.GetElementSpaceSize());
    DeviceMem b_device_buf(sizeof(BDataType) * b.mDesc.GetElementSpaceSize());

    a_device_buf.ToDevice(a.mData.data());
    // LogRangeAsType<float>(std::cout << "Tensor a  : ", a.mData, ",") << std::endl;

    std::array<const void*, 1> input = {a_device_buf.GetDeviceBuffer()};
    std::array<void*, 1> output      = {b_device_buf.GetDeviceBuffer()};

    std::array<ck::index_t, 4> ab_lengths;
    std::array<ck::index_t, 4> a_strides = {static_cast<int>(nchw[1] * nchw[2] * nchw[3]),
                                            static_cast<int>(nchw[2] * nchw[3]),
                                            static_cast<int>(nchw[3]),
                                            1};
    std::array<ck::index_t, 4> b_strides = {static_cast<int>(nhwc[1] * nhwc[2] * nhwc[3]),
                                            1,
                                            static_cast<int>(nhwc[2] * nhwc[3]),
                                            static_cast<int>(nhwc[3])};

    std::copy(nchw.begin(), nchw.end(), ab_lengths.begin());

    auto broadcastPermute = DeviceElementwisePermuteInstance{};
    auto argument         = broadcastPermute.MakeArgumentPointer(
        ab_lengths, {a_strides}, {b_strides}, input, output, PassThrough{});

    if(!broadcastPermute.IsSupportedArgument(argument.get()))
    {
        throw std::runtime_error(
            "The runtime parameters seems not supported by the device instance, exiting!");
    };
    auto broadcastPermute_invoker_ptr = broadcastPermute.MakeInvokerPointer();
    float ave_time =
        broadcastPermute_invoker_ptr->Run(argument.get(), StreamConfig{nullptr, time_kernel});

    std::cout << "Perf: " << ave_time << " ms" << std::endl;

    bool pass = true;

    if(do_verification)
    {
        b_device_buf.FromDevice(b.mData.data());
        // LogRangeAsType<float>(std::cout << "Tensor b  : ", b.mData, ",") << std::endl;
        Tensor<BDataType> host_b(nhwc);
        host_elementwise4D(host_b, a, PassThrough{});

        // LogRangeAsType<float>(std::cout << "Host b  : ", host_b.mData, ",") << std::endl;
        pass &=
            ck::utils::check_err(b.mData, host_b.mData, "Error: Incorrect results b", 1e-3, 1e-3);
    }

    return pass ? 0 : 1;
}
