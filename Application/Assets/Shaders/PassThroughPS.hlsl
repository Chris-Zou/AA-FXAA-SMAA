Texture2D backBuffer : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 PS_PassThrough(PSIn _input) : SV_TARGET
{
    return backBuffer.Sample(Sampler, _input.TexCoord);
}
