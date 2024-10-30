struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float2 uv : TEXCOORD;
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
    result.uv = input.uv;

    return result;
}