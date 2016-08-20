Texture2D backBuffer : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 PS_FXAALUMA(PSIn _input) : SV_TARGET
{
	float4 color = backBuffer.Sample( Sampler, _input.TexCoord);
	color.a = dot(color.xyz, float3(0.299, 0.587, 0.114));
	return color;
}
