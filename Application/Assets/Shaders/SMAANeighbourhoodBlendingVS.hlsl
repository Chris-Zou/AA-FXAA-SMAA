#define SMAA_HLSL_4 1
#define PS_VERSION ps_4_1
#define SAMPLERS_DEFINED_OUTSIDE

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
	float4 Offset[2] : TEXCOORD2;
};

VSOut VS_SMAANeighbourhoodBlending(VSIn _input)
{
	VSOut output;
	
	output.Position = float4(_input.Position, 1.f);
	output.TexCoord = _input.TexCoord;
	
	SMAANeighborhoodBlendingVS(float4(_input.Position, 1.f), output.Position, output.TexCoord, output.Offset);
	
    return output;
}
