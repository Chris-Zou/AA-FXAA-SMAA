Texture2D backBuffer : register(t0);
#define SMAA_HLSL_4 1
#define PS_VERSION ps_4_1
#define SMAA_PRESET_HIGH 1

//SamplerState LinearSampler  : register( s0 ); // { Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
//SamplerState PointSampler   : register( s1 ); // { Filter = MIN_MAG_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };


cbuffer SMAABuffer
{
	float2 TexelSize;
	float2 dummy;
};

#define SMAA_PIXEL_SIZE TexelSize

#include "Assets/Shaders/SMAA.h"

struct PSIn
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD;
	float4 Offset[3] : TEXCOORD2;
};

float4 PS_SMAAEdgeDetection(PSIn _input)  : SV_TARGET
{	
	return SMAALumaEdgeDetectionPS(_input.TexCoord, _input.Offset, backBuffer);
}