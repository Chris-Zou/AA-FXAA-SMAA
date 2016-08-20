Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState Sampler : register(s0);

cbuffer LightBuffer
{
	float4 DiffuseColor;
    float3 LightDirection;
	float pad;
};

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 Tex 		: TEXCOORD;
};

float4 PS_Light(PSIn _input) : SV_Target0
{
	float4 textureColor;
	float4 normals;
	float3 lightDir;
	float lightIntensity;
	float4 color;
	

	// Sample the texture pixel at this location.
	textureColor = colorTexture.Sample(Sampler, _input.Tex);
	
	// Sample the normals from the normal render texture using the point sampler at this texture coordinate location.
	normals = normalTexture.Sample(Sampler, _input.Tex);
	
	// Invert the light direction for calculations.
	lightDir = -LightDirection;

	// Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(normals, lightDir));

	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
	color = saturate(DiffuseColor * lightIntensity);

	// Multiply the texture pixel and the input color to get the final result.
	color = saturate(textureColor*lightIntensity);

    return color;
}
