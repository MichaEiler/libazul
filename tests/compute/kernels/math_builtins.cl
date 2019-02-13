__kernel void kernel_math_builtins(__global float *input)
{
    const size_t index = get_global_id(0);

    input[index] = cos(input[index]);
    input[index] = acos(input[index]);
    input[index] = sin(input[index]);
    input[index] = asin(input[index]);
    input[index] = tan(input[index]);
    input[index] = cosh(input[index]);
    input[index] = acosh(input[index]);
    input[index] = sinh(input[index]);
    input[index] = asinh(input[index]);
    input[index] = tanh(input[index]);
    input[index] = atanh(input[index]);
    input[index] = floor(input[index]);
    input[index] = ceil(input[index]);
    input[index] = round(input[index]);
    input[index] = exp(input[index]);
    input[index] = exp2(input[index]);
    input[index] = log(input[index]);
    input[index] = log2(input[index]);
    input[index] = log10(input[index]);
    input[index] = sqrt(input[index]);
    input[index] = abs(input[index]);
    input[index] = pow(input[index], 2.0f);
    input[index] = atan2(input[index], 2.0f);

    const float a = cos(input[index]);
    const float b = sin(input[index]);
    input[index] = max(a, b) + min(a, b);
}
