cbuffer WVPBuffer
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

struct VSIn
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VSOut
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VSOut VS_Light(VSIn _input)
{
    VSOut output;

	output.Position = float4(_input.Position, 1.f);
	/*
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
	*/
	
    output.TexCoord = _input.TexCoord;
    
	return output;
}