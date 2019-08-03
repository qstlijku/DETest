#include "Common.hlsli"

struct VertexShaderInput {
	float2 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 color : COLOR;
};

struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 color : COLOR;
};

PixelShaderInput main(VertexShaderInput input) {
	PixelShaderInput output;

	// Map to normalized clip coordinates:
	float x = ((2.0 * (input.pos.x - 0.5)) / WindowSize.x) - 1.0;
	float y = 1.0 - ((2.0 * (input.pos.y - 0.5)) / WindowSize.y);

	output.pos = float4(x, y, 0, 1);
	output.uv = input.uv;
	output.color = input.color;

	return output;
}