struct PSInput
{
    float2 uv : TEXCOORD;
};

Texture2D AlbedoTexture : register(t0);
SamplerState AlbedoSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    return AlbedoTexture.Sample(AlbedoSampler, input.uv);
}