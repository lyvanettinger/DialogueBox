struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 color : COLOR;
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
    result.color = input.color;

    return result;
}