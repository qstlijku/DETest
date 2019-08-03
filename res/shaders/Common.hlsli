cbuffer SceneConstantBuffer : register(b0) {
	matrix View;
	matrix Projection;
	matrix ViewProjection;
	float2 WindowSize;
};

cbuffer ObjectConstantBuffer : register(b1) {
	matrix Model;
	float2 UVOffset;
	float2 UVScale;
};
