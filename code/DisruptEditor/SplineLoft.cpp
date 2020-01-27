#include "SplineLoft.h"

void SplineLoftHiRes::open(IBinaryArchive& fp) {
	fp.serializeConstant<uint32_t>(1397508178);

	fp.serializeConstant<uint32_t>(7);

	fp.serialize(unk1);//Unused
	
	fp.serializeNdVector(vertexData);
	fp.serializeNdVector(indexData);
	createBuffers();

	fp.markHeader();
	fp.markInPlaceOffset(fp.header.unk2);

	fp.serializeNdVectorExternal(networkRegionResources, "CSplineNetworkRegionResourceEntry");

	fp.serialize(lowRes);
	fp.finish();
}

void SplineLoftHiRes::draw(ID3D11DeviceContext* context) {
	RenderInterface::instance().objectCB.Offset = glm::vec4(1, 0.5, 0.5, 0);
	RenderInterface::instance().objectCB.Model = glm::mat4(1);
	context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

	context->VSSetShader(RenderInterface::instance().spline.pVertexShader, NULL, NULL);
	context->PSSetShader(RenderInterface::instance().spline.pPixelShader, NULL, NULL);

	D3D_PRIMITIVE_TOPOLOGY pType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;// D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	context->IASetPrimitiveTopology(pType);

	//loadMaterial(materialResources.materials[mesh.matID].file.c_str())->bind(context);

	UINT offset = 0;
	UINT stride = sizeof(UnkStr);
	context->IASetVertexBuffers(0, 1, &vertex->pVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(index->pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
	if (!layout.Get()) {
		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			//DXGI_FORMAT_R32G32B32_SINT
		  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "TEXCOORD", 0, DXGI_FORMAT_R16G16B16A16_SINT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		HRESULT ret = RenderInterface::instance().g_pd3dDevice->CreateInputLayout(
			vertexDesc,
			ARRAYSIZE(vertexDesc),
			RenderInterface::instance().spline.vShaderBlob->GetBufferPointer(),
			RenderInterface::instance().spline.vShaderBlob->GetBufferSize(),
			&layout);
		SDL_assert_release(ret == S_OK);
	}
	context->IASetInputLayout(layout.Get());

	context->Draw(vertexData.size(), 0);
	//context->DrawIndexed(indexData.size(), 0, 0);
}

void SplineLoftHiRes::createBuffers() {
	vertex = std::make_shared<VertexBuffer>();
	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = vertexData.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(vertexData.size() * sizeof(UnkStr), D3D11_BIND_VERTEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertex->pVertexBuffer);

	index = std::make_shared<IndexBuffer>();
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indexData.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(indexData.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&index->pIndexBuffer);
	index->size = indexData.size() * sizeof(uint16_t);
}

void LoftShape::open(IBinaryArchive& fp) {
	uint32_t magic = 1280263764;
	fp.serialize(magic);
	SDL_assert_release(magic == 1280263764);

	uint32_t version = 24;
	fp.serialize(version);
	SDL_assert_release(version == 24);
}

void CSceneSplineLoftRegion::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serializeNdVectorExternal(splines, "CSpline");

	fp.serialize(unk4);
	fp.PreAllocateSizeOfType(0xA1A47E4E, unk4);//Something Like CSceneObjectHandle<CSceneMaterial> *

	fp.serialize(unk5);
	fp.PreAllocateSizeOfType("CSplineLoftElementDBInfo", unk5);

	static_assert(sizeof(SSplineRangeCoords) == (1 << 4));
	fp.serializeNdVectorInPlace(rangeCoords);

	static_assert(sizeof(CRangeOffsets) == 0x24);
	fp.serializeNdVectorInPlace(rangeOffsets);

	fp.serializeNdVectorExternal(morphing, "CSplineLoftMorphing");

	static_assert(sizeof(CSplineLoftMeshDesc) == 0x8);
	fp.serializeNdVectorInPlace(meshDescs);

	static_assert(sizeof(CSplineLoftMeshLODGFXDesc) == 2);
	fp.serializeNdVectorInPlace(meshLODGFXDescs);

	fp.serializeNdVectorInPlace(unk6);

	fp.serializeNdVectorInPlace(unk7);

	static_assert(sizeof(CSplineLoftPrimitiveDrawCallDesc) == 4);
	fp.serializeNdVectorInPlace(primitiveDrawCallDesc);

	static_assert(sizeof(CSplineLoftPrimitiveDrawCallGFXBuffers) == 0x10);
	fp.serializeNdVectorInPlace(primitiveDrawCallGFXBufDesc);
}

void CSplineNetworkRegionResourceEntry::read(IBinaryArchive& fp) {
	fp.PreAllocateSizeOfType("CSceneSplineLoftRegion", 1);

	fp.serialize(loftReigon);
	fp.serializeNdVectorExternal(primitiveDesc, "CSplineLoftPrimitiveDesc");
	fp.serializeNdVectorExternal(drawCalls, "SSplineLoftDrawCall");
	fp.serializeNdVector(unk2);
	fp.serializeNdVectorInPlace(unk3);
}

void CSpline::read(IBinaryArchive& fp) {
	fp.serialize(splineName);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);

	fp.serializeNdVectorInPlace(controlPoints);
}

void CSplineControlPoint::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
}

void CSplineLoftPrimitiveDesc::read(IBinaryArchive& fp) {
	fp.PreAllocateSizeOfType("CSceneSplineLoftPrimitive", 1);

	fp.PreAllocateSizeOfType("CSceneSplineLoftPrimitive", 1);

	//fp.PreAllocateSizeOfType("CSceneSplineLoftPrimitive", 1);

	fp.serialize(primitive);

	//TODO: SerializeBasicTypeInPlace(0,0x50,1,0x10,1);
	fp.memBlockInPlace(unk2.data(), sizeof(unk2[0]), unk2.size());
}

void SSplineLoftDrawCall::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);

	//TODO: SerializeInPlace__tm__3_Uc
	//TODO: SerializeInPlace__tm__3_Us
}

void CSplineLoftMorphing::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serializeNdVectorExternal(knots, "CKnot");
}

void CKnot::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(type);
}

void CSceneSplineLoftPrimitive::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
}
