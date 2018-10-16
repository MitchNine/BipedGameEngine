
cbuffer cbPerObject_gizmos
{
	float4x4 WVP;
    float4x4 World;
	
	float4 color;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float4 Color : COLOR;
};

VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 normal : NORMAL, float3 tangent : TANGENT)
{
    VS_OUTPUT output;
	
    output.Pos = mul(inPos, WVP);
	output.worldPos = mul(inPos, World);
	output.normal = mul(normal, World);
	output.tangent = mul(tangent, World);
    output.TexCoord = inTexCoord;
	output.Color = color;
	
    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return input.Color;
}