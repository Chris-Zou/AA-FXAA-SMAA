Texture2D backBuffer : register(t0);
SamplerState Sampler : register(s0);

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 39

#include "Assets/Shaders/Fxaa3_11.h"

cbuffer SMAABuffer
{
	float2 rcpFrame;
	float4 rcpFrameOpt;
};

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 PS_FXAA(PSIn _input) : SV_TARGET
{
	int width = 0;
	int height = 0;
	backBuffer.GetDimensions(width, height);
	
	float qualitySubpixels = 0.75f;
	float qualityEdgeThreshold = 0.125f;
	float qualityedgeThresholdMin = 0.0312f;
	
	FxaaTex tex;
    tex.smpl = Sampler;
    tex.tex  = backBuffer;

	float2 screenPos = (_input.TexCoord.xy);
	return FxaaPixelShader( screenPos, float4( 0, 0, 0, 0 ), 
	tex, tex, tex, 
	rcpFrame,
	rcpFrameOpt, float4( 0, 0, 0, 0), float4( 0, 0, 0, 0), 
	qualitySubpixels, qualityEdgeThreshold, qualityedgeThresholdMin, 
	0, 0, 0, 
	float4( 0, 0, 0, 0));
}