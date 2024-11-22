struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float2 uv : TEXCOORD;
    float3 normal : NORMAL0;
    float4 position : SV_POSITION;
};

cbuffer ModelViewProjectionCB : register(b0)
{
    matrix MVP;
};

VSOutput main(VSInput input)
{
    VSOutput result;

    result.position = mul(MVP, float4(input.position, 1.0f));
    result.normal = input.normal; // TODO: multiply with inverse transpose
    result.uv = input.uv;

    return result;
}