Texture2D backBuffer : register(t0);
SamplerState Sampler : register(s0);

/*
cbuffer FXAASettings
{

};*/

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
	int width = 0;
	int height = 0;
	backBuffer.GetDimensions(width, height);
	
	//fxaaQualityRcpFrame == Texelsize. Textures are represented by an array of texels.
	float2 texelSize;
	texelSize.x = 1.0f/(float)width;
	texelSize.y = 1.0f/(float)height;
	float4 rgbM  = backBuffer.Sample(Sampler, _input.TexCoord + (float2(0.0f, 0.0f) * texelSize));
	if(_input.TexCoord.x < 0.6f && _input.TexCoord.x > 0.4f)
	{
		if(_input.TexCoord.y > 0.4f && _input.TexCoord.y < 0.6f)
		{
			float4 rgbT  = backBuffer.Sample(Sampler, _input.TexCoord + (float2(-1.0f, -1.0f) * texelSize));
			return rgbT;
		}
	}
	
	return rgbM;	
}
