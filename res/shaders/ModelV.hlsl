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

	float4 modelPos = input.pos;
	modelPos *= Offset.y;
	modelPos += Offset.x;
	modelPos.w = 1.0f;
	output.pos = mul(mul(ViewProjection, Model), modelPos);

	float2 uvPos = input.uv;
	output.uv = uvPos * Offset.w + Offset.z;

	return output;
}