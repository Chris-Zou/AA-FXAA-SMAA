Texture2D backBuffer : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 PS_FXAA(PSIn _input) : SV_TARGET
{
	//We have Position
	//We have tex coordinates.
	
	int width = 0;
	int height = 0;
	backBuffer.GetDimensions(width, height);
	
	//fxaaQualityRcpFrame == Texelsize. Textures are represented by an array of texels.
	float2 texelSize;
	texelSize.x = 1.0f/(float)width;
	texelSize.y = 1.0f/(float)height;
	
	//fxaaQualityEdgeThreshold - skulle kunna vara i constant buffern.
	/*
	// The minimum amount of local contrast required to apply algorithm.
    //   0.333 - too little (faster)
    //   0.250 - low quality
    //   0.166 - default
    //   0.125 - high quality 
    //   0.063 - overkill (slower)
	*/
	float fxxaQualityEdgeThresholdMax = 0.125;
	
	//fxaaQualityEdgeThresholdMin
	/*
	// Trims the algorithm from processing darks.
    //   0.0833 - upper limit (default, the start of visible unfiltered edges)
    //   0.0625 - high quality (faster)
    //   0.0312 - visible limit (slower)
    // Special notes when using FXAA_GREEN_AS_LUMA,
    //   Likely want to set this to zero.
    //   As colors that are mostly not-green
    //   will appear very dark in the green channel!
    //   Tune by looking at mostly non-green content,
    //   then start at zero and increase until aliasing is a problem.
	*/
	float fxaaQualityEdgeThresholdMin = 0.0625f;


	
	//We check the luminance of the pixel and around the pixel.
	//
	
	
	const float3 lumaConstant = float3(0.299f, 0.587, 0.114);
	//Corners
	float lumaNW = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(-1.0,	-1.0) 	* texelSize)).xyz);
	float lumaNE = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(1.0,	-1.0)	* texelSize)).xyz);
	float lumaSW = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(-1.0,	1.0)	* texelSize)).xyz);
	float lumaSE = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(1.0,	1.0)	* texelSize)).xyz);
	//Middle
	float lumaM =  dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord).xyz);
	
	float rangeMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float rangeMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
	//Edge detection
	//lumaMin and lumaMax through range
	float range = rangeMax - rangeMin;
	if(range < max(fxaaQualityEdgeThresholdMin , rangeMax * fxxaQualityEdgeThresholdMax))
	{
		return backBuffer.Sample(Sampler, _input.TexCoord);
	}
	
	//Direction
	float2 direction;
	direction.x = - lumaNW - lumaNE + lumaSW + lumaSE;
	direction.y =  lumaNW + lumaSW - lumaNE - lumaSE;
	
	float directionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * fxxaQualityEdgeThresholdMax), fxaaQualityEdgeThresholdMin);
	float rcpDirectionMin = 1.0f/(min(abs(direction.x), abs(direction.y))+directionReduce);
	
	float FXAA_SPAN_MAX = 8.0f;
	direction = min(float2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
				max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), direction * rcpDirectionMin)
				) * texelSize;

	float3 result = 0.5f * (
		backBuffer.Sample(Sampler, _input.TexCoord + (direction * float2((-1.0f / 6.0f), (-1.0f / 6.0f)))) +
		backBuffer.Sample(Sampler, _input.TexCoord + (direction * float2((1.0f / 6.0f), (1.0f / 6.0f))))).xyz;
	
	float3 result2 = result * 0.5f + (0.25f) * (
		backBuffer.Sample(Sampler, _input.TexCoord + (direction * float2(-0.5f, - 0.5f))) +
		backBuffer.Sample(Sampler, _input.TexCoord + (direction * float2( 0.5f,   0.5f)))).xyz;
	
	
	float lumaResult = dot(lumaConstant, result2);
	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (lumaResult < rangeMin || lumaResult > rangeMax)
	{
		finalColor = float4(result, 1.0f); //närliggande
	}
	else
	{
		finalColor = float4(result2, 1.0f);
	}
	
	return finalColor;
}
