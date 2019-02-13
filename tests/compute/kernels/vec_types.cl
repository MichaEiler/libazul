#ifdef __OPENCL_VERSION__
#define make_float4(a, b, c, d) (float4)(a, b, c, d)
#endif // __OPENCL_VERSION__

__kernel void kernel_vec_types(__global float* input)
{
    const size_t index = get_global_id(0);

    float4 data = make_float4(input[index], input[index], input[index], input[index]);
    data *= 2.0f;

    input[index] = data.x + data.y + data.z + data.w;
}