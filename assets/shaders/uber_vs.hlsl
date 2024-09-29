struct VSInput
{
    float4 color : COLOR;
    float3 position : POSITION;
};

struct VSOutput
{
    float4 color : COLOR;
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput result;

    result.position = float4(input.position, 1.0f);
    result.color = input.color;

    return result;
}