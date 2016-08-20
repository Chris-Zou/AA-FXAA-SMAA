Texture2D sceneTexture : register(t0);
Texture2D edgesTexture : register(t1);
Texture2D areaTexture : register(t2);
Texture2D searchTexture : register(t3);

//Constants for the area texture that is given by SMAA
#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / float2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20

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
	float2 Texel 	: TEXCOORD2;
	float2 PixCoord : TEXCOORD3;
	float4 Offset[3]: TEXCOORD4;
};

float4 PS_SMAABlendingWeight(PSIn _input)  : SV_TARGET
{	
	return SMAABlendingWeightCalculationPS(
	_input.TexCoord, 
	_input.PixCoord, 
	_input.Offset, 
	edgesTexture, 
	areaTexture, 
	searchTexture, 0);
}
