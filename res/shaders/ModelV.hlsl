#include "Common.hlsli"

struct VertexShaderInput {
	int4 pos : POSITION;
	int2 uv : TEXCOORD0;
};

struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input) {
	PixelShaderInput output;

	float4 modelPos = float4(input.pos.xyz * (1.0 / 32767.0) * Offset.x, 1.0);
	output.pos = mul(mul(ViewProjection, Model), modelPos);
	output.uv = input.uv * (1.0 / 32767.0) * Offset.y + Offset.z;

	return output;
}