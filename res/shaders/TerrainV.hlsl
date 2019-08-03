#include "Common.hlsli"

struct VertexShaderInput {
	float4 pos : POSITION;
};

struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input) {
	PixelShaderInput output;

	output.pos = mul(mul(ViewProjection, Model), input.pos);
	output.uv = ((input.pos.xy / 68.0) / 4.0) + UVOffset;

	return output;
}