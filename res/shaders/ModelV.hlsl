#include "Common.hlsli"

struct VertexShaderInput {
	float4 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input) {
	PixelShaderInput output;

	output.pos = mul(mul(ViewProjection, Model), input.pos * UVScale.x);
	output.uv = input.uv;

	return output;
}