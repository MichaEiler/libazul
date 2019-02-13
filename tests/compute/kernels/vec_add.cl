__kernel void kernel_vec_add(__global float* inputA, __global float* inputB, __global float* result)
{
    const size_t index = get_global_id(0);
    result[index] = inputA[index] + inputB[index];
}