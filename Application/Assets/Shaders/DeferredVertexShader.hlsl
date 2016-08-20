cbuffer WVPBuffer
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

struct VSIn
{
    float3 Position : POSITION;
	float3 Normal 	: NORMAL;
    float2 Tex 		: TEXCOORD;
};

struct VSOut
{
    float4 Position : SV_POSITION;
	float3 Normal 	: NORMAL;
    float2 Tex 		: TEXCOORD;
};

VSOut VS_Deferred(VSIn _input)
{
    VSOut output = (VSOut)0;
    
	// Change the position vector to be 4 units for proper matrix calculations.
    output.Position = float4(_input.Position, 1.f);

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
	// Store the texture coordinates for the pixel shader.
	output.Normal = mul(_input.Normal, (float3x3)World);
	output.Normal = normalize(output.Normal);
	
	output.Tex = _input.Tex;
	
    
    return output;
}