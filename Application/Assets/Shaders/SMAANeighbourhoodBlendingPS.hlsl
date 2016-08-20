Texture2D backBuffer : register(t0);
Texture2D blendTexture : register(t1);

#define SMAA_HLSL_4 1
#define PS_VERSION ps_4_1
#define SMAA_PRESET_HIGH 1
cbuffer SMAABuffer
{
	float2 TexelSize;
	float2 dummy;
};

#define SMAA_PIXEL_SIZE TexelSize

#include "Assets/Shaders/SMAA.h"

struct PSIn
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
	float4 Offset[2] : TEXCOORD2;
};

float4 PS_SMAANeighbourhoodBlending(PSIn _input)  : SV_TARGET
{	
	return SMAANeighborhoodBlendingPS(_input.TexCoord, _input.Offset, backBuffer, blendTexture);
}
