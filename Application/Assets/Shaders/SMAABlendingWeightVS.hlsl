#define SMAA_HLSL_4 1
#define PS_VERSION ps_4_1

cbuffer SMAABuffer
{
	float2 TexelSize;
	float2 dummy;
};

#define SMAA_PIXEL_SIZE TexelSize

#include "Assets/Shaders/SMAA.h"

struct VSIn
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VSOut
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
	float2 Texel 	: TEXCOORD2;
	float2 PixCoord : TEXCOORD3;
	float4 Offset[3]: TEXCOORD4;
};

VSOut VS_SMAABlendingWeight(VSIn _input)
{
	VSOut output;
	
	output.Position = float4(_input.Position, 1.f);
	output.TexCoord = _input.TexCoord;
	output.Texel = TexelSize;
	
	SMAABlendingWeightCalculationVS(
	float4(_input.Position, 1.f), 
	output.Position, 
	output.TexCoord, 
	output.PixCoord, 
	output.Offset);
	
    return output;;
}
