struct PSInput
{
    float2 uv : TEXCOORD;
    float3 normal : NORMAL0;
};

Texture2D AlbedoTexture : register(t0);
SamplerState AlbedoSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    return float4(input.uv.x, input.uv.y, 0.0, 1.0);
    //return AlbedoTexture.Sample(AlbedoSampler, input.uv);
}