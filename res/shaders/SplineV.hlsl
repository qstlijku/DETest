#include "Common.hlsli"

struct VertexShaderInput {
	float4 pos : POSITION;
	int2 uv : TEXCOORD0;
};

struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input) {
	PixelShaderInput output;

	output.pos = mul(mul(ViewProjection, Model), input.pos);
	output.uv = input.uv * (1.0 / 32767.0);

	return output;
}