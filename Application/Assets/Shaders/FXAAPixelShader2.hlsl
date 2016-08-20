Texture2D backBuffer : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float FxaaLuma(float4 _rgb)
{
	return _rgb.y * 1.9632107f/*(0.587/0.299) */+ _rgb.x;
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
	float4 rgbM = backBuffer.Sample(Sampler, _input.TexCoord);
	
	const float3 lumaConstant = float3(0.299f, 0.587, 0.114);
	//Corners
	float lumaM =  dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord).xyz);
	float lumaNW = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(-1.0,	-1.0) 	* texelSize)).xyz);
	float lumaNE = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(1.0,	-1.0)	* texelSize)).xyz);
	float lumaSW = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(-1.0,	1.0)	* texelSize)).xyz);
	float lumaSE = dot(lumaConstant, backBuffer.Sample(Sampler, _input.TexCoord + (float2(1.0,	1.0)	* texelSize)).xyz);
	
	
	
	lumaNE += (1.0f /384.0f);
	//Remove below
	max(lumaNW, lumaSW);
	min(lumaNW, lumaSW);
	max(lumaNE, lumaSE);
	min(lumaNE, lumaSE);
	//
	float lumaMax = max(max(lumaNE, lumaSE), max(lumaNW, lumaSW));
	float lumaMin = min(min(lumaNE, lumaSE), min(lumaNW, lumaSW));
	float lumaMaxScaled = lumaMax * 0.125f; //0.125 == 1/8 == fxaaConsoleEdgeThreshold
	
	float lumaMaxSubMinM = max(lumaMax, lumaM) - min(lumaMin, lumaM);
	float lumaMaxScaledClamped = max(0.04, lumaMaxScaled); // 0.05 == fxaaConsoleEdgeThresholdMin - 0.04- 0.6 where 0.4 is less aliasing
	
	if(lumaMaxSubMinM < lumaMaxScaledClamped)
	{
		return rgbM;
	}
	
	////
	float2 dir;
	dir.x = lumaSW - lumaNE + lumaSE - lumaNW;
	dir.y = lumaSW - lumaNE - lumaSE + lumaNW;
	float2 dir1 = normalize(dir.xy);
	
	//
	float fxxaQualityEdgeThresholdMax = 0.166f;
	float fxaaQualityEdgeThresholdMin = 0.0625f;
	float directionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * fxxaQualityEdgeThresholdMax), fxaaQualityEdgeThresholdMin);
	//
	
	float4 rgbyN1 = backBuffer.Sample(Sampler, _input.TexCoord - dir1 	* texelSize);
	float4 rgbyP1 = backBuffer.Sample(Sampler, _input.TexCoord + dir1 	* texelSize);
	
	float dirAbsMinTimeC = min(abs(dir1.x), abs(dir1.y)) * 8;
	
	float2 dir2 = clamp(dir1.xy / dirAbsMinTimeC, -2.0f, 2.0f);
	float4 rgbyN2 = backBuffer.Sample(Sampler, _input.TexCoord - dir2 	* texelSize);
	float4 rgbyP2 = backBuffer.Sample(Sampler, _input.TexCoord + dir2 	* texelSize);
	float4 rgbyA = rgbyN1 + rgbyP1;
	float4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);
	
	if( (rgbyB.w < lumaMin) || (rgbyB.w > lumaMax))
	{
		return rgbyA*0.5f;
	}
	else
	{
		return rgbyB;
	}
	
	return float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	
	
	
	
	
	
	
	
}
