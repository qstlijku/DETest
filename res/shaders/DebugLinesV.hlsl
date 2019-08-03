#include "Common.hlsli"

struct VertexShaderInput {
	float4 pos : POSITION;
	float3 color : COLOR;
};

struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};

PixelShaderInput main(VertexShaderInput input) {
	PixelShaderInput output;

	output.pos = mul(ViewProjection, input.pos);
	output.color = input.color;

	return output;
}