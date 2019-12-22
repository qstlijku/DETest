struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D    colorTexture : register(t0);
SamplerState colorSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return colorTexture.Sample(colorSampler, input.uv.xy);
}