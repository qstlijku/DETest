struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 color : COLOR;
};

Texture2D    colorTexture : register(t0);
SamplerState colorSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(input.color, colorTexture.Sample(colorSampler, input.uv.xy).r);
}