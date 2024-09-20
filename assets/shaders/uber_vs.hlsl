struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR) : SV_TARGET
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}