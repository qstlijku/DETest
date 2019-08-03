struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D    colorTexture : register(t0);
SamplerState colorSampler : register(s0);

Texture2D    diffuseTexture : register(t1);
SamplerState diffuseSampler : register(s1);

Texture2D    maskTexture : register(t2);
SamplerState maskSampler : register(s2);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return diffuseTexture.Sample(diffuseSampler, input.uv.xy);
}