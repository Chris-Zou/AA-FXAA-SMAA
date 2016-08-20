/*--------------------------------------------------------------------------------------------------------------------
| Pixel shader that applies FXAA to the backbuffer-copy, and then renders to the real backbuffer. 				     |
--------------------------------------------------------------------------------------------------------------------*/

Texture2D backbuffertemp : register(t0);
SamplerState samplerWrap : register(s0);

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 PS_FXAA(VS_OUT input) : SV_TARGET
{
	float4 finalColor = float4(0,0,0,0);

	//Apply FXAA
	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MIN = 1.0f / 128.0f;
	float FXAA_REDUCE_MUL = 1.0f / 8.0f;

	int width = 0;
	int height = 0;
	backbuffertemp.GetDimensions(width, height);
	float2 texelSize = float2(1.0f / width, 1.0f / height);

	float3 luma = float3(0.299f, 0.587, 0.114);
	float lumaTL = dot(luma, backbuffertemp.Sample(samplerWrap, input.uv + (float2(-1.0f, 1.0f) * texelSize)));
	float lumaTR = dot(luma, backbuffertemp.Sample(samplerWrap, input.uv + (float2(1.0f, 1.0f) * texelSize)));
	float lumaBL = dot(luma, backbuffertemp.Sample(samplerWrap, input.uv + (float2(-1.0f, -1.0f) * texelSize)));
	float lumaBR = dot(luma, backbuffertemp.Sample(samplerWrap, input.uv + (float2(1.0f, -1.0f) * texelSize)));
	float lumaM = dot(luma, backbuffertemp.Sample(samplerWrap, input.uv).xyz);
		
	
	float2 dir;
	dir.x = ((lumaTL + lumaTR) - (lumaBL + lumaBR));
	dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

	float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN);
	float inverseDirAdj = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);

	dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
		max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * inverseDirAdj)) * texelSize;

	float3 result1 = 0.5 * (
		backbuffertemp.Sample(samplerWrap, input.uv + (dir * float2((1.0f / 3.0f) - 0.5f, (1.0f / 3.0f) - 0.5f))) +
		backbuffertemp.Sample(samplerWrap, input.uv + (dir * float2((2.0f / 3.0f) - 0.5f, (2.0f / 3.0f) - 0.5f)))).xyz;
	
	
	float3 result2 = result1 * 0.5 + (1.0f / 4.0f) * (
		backbuffertemp.Sample(samplerWrap, input.uv + (dir * float2((0.0f / 3.0f) - 0.5f, (0.0f / 3.0f) - 0.5f))) +
		backbuffertemp.Sample(samplerWrap, input.uv + (dir * float2((3.0f / 3.0f) - 0.5f, (3.0f / 3.0f) - 0.5f)))).xyz;
		
	float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
	float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
	float lumaResult2 = dot(luma, result2);

	if (lumaResult2 < lumaMin || lumaResult2 > lumaMax)
	{
		finalColor = float4(result1, 0.0f);
	}
	else
	{
		finalColor = float4(result2, 0.0f);
	}

	return finalColor;
}