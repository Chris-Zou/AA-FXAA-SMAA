//b#: A register for a constant buffer (cbuffer).
//t#: A register for a texture buffer (tbuffer).
//s#: A register for a sampler. (A sampler defines the lookup behavior for texels in the texture resource.)

Texture2D shaderTexture : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
    float4 Position : SV_POSITION;
	float3 Normal 	: NORMAL;
    float2 Tex 		: TEXCOORD;
};

struct PSOut
{
	float4 Color 	: SV_Target0;
    float4 Normal 	: SV_Target1;
};

PSOut PS_Deferred(PSIn _input)
{
	PSOut output;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    output.Color 	= shaderTexture.Sample(Sampler, _input.Tex);
	output.Normal 	= float4(_input.Normal, 1.0f);

    return output;
}
