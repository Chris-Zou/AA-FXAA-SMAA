struct VSIn
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VSOut
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VSOut VS_PassThrough(VSIn _input)
{
	VSOut output;
	
	output.Position = float4(_input.Position, 1.f);
	output.TexCoord = _input.TexCoord;
	
    return output;
}
