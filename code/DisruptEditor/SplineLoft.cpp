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
		  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "TEXCOORD2", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "NORMAL", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "TEXCOORD1", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "TEXCOORD0", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	fp.serializeNdVectorInPlace(rangeCoords, 4);

	static_assert(sizeof(CRangeOffsets) == 0x24);
	fp.serializeNdVectorInPlace(rangeOffsets, 4);

	fp.serializeNdVectorExternal(morphing, "CSplineLoftMorphing");

	static_assert(sizeof(CSplineLoftMeshDesc) == 0x8);
	fp.serializeNdVectorInPlace(meshDescs, 4);

	static_assert(sizeof(CSplineLoftMeshLODGFXDesc) == 2);
	fp.serializeNdVectorInPlace(meshLODGFXDescs, 4);

	fp.serializeNdVectorInPlace(unk6, 4);

	fp.serializeNdVectorInPlace(unk7, 8);

	static_assert(sizeof(CSplineLoftPrimitiveDrawCallDesc) == 4);
	fp.serializeNdVectorInPlace(primitiveDrawCallDesc, 4);

	static_assert(sizeof(CSplineLoftPrimitiveDrawCallGFXBuffers) == 0x10);
	fp.serializeNdVectorInPlace(primitiveDrawCallGFXBufDesc, 4);
}

void CSplineNetworkRegionResourceEntry::read(IBinaryArchive& fp) {
	fp.PreAllocateSizeOfType("CSceneSplineLoftRegion", 1);

	fp.serialize(loftReigon);
	fp.serializeNdVectorExternal(primitiveDesc, "CSplineLoftPrimitiveDesc");
	fp.serializeNdVectorExternal(drawCalls, "SSplineLoftDrawCall");
	SDL_assert_release(drawCalls.size() == 0);
	fp.serializeNdVector(materials);
	fp.serializeNdVectorInPlace(unk3, 4);
}

void CSpline::read(IBinaryArchive& fp) {
	fp.serialize(splineName);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);

	fp.serializeNdVectorInPlace(controlPoints, 0x10);
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

	//TODO: SerializeBasicTypeInPlace(ptr: 0, objCount: 0x50, objSize: 1, padding: 0x10, DoInPlace: 1);
	fp.padInPlace(0x10);
	fp.memBlockInPlace(unk2.data(), 1, unk2.size());
}

void SSplineLoftDrawCall::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);

	uint32_t unk3 = unk5.size();
	fp.serialize(unk3);

	uint32_t unk4 = unk6.size();
	fp.serialize(unk4);

	fp.serializeInPlace(unk5, unk3, 0x10);

	fp.serializeInPlace(unk6, unk4, 0x10);
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

void SplineLoftLowRes::open(IBinaryArchive& fp) {
	fp.serializeConstant<uint32_t>(1397509202);
	fp.serializeConstant<uint32_t>(5);
	fp.serialize(unk1);
	fp.serializeConstant<uint32_t>(0);

	fp.markHeader();
	fp.PauseInPlace();

	uint32_t count = materials.size();
	materials.resize(count);
	fp.PreAllocateMemory(count << 2, 4);
	for (auto& it : materials)
		fp.serialize(it);

	fp.PreAllocateSizeOfType(0xA1A47E4E, materials.size());
	fp.serialize(unk3);
	fp.serialize(hiRes);
	fp.serialize(vertexDataSize);
	fp.serialize(indexDataSize);

	fp.finish();

	vertexData.resize(vertexDataSize);
	fp.memBlock(vertexData.data(), 1, vertexData.size());

	indexData.resize(indexDataSize);
	fp.memBlock(indexData.data(), 1, indexData.size());
}

void CSceneSplineLoftBatch::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serializeNdVector(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serializeNdVector(unk11);
	fp.serializeNdVector(unk12);
	fp.serializeNdVector(unk13);
}

void CSceneSplineLoftBatch::SRangeDesc::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
}

void CSceneSplineLoftBatch::SPassDrawCallRanges::read(IBinaryArchive& fp) {
	for (int i = 0; i < unk1.size(); ++i)
		fp.serialize(unk1[i]);
}

void SArrayRange::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void CSceneSplineLoftBatch::SPrimitiveData::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	for (int i = 0; i < unk2.size(); ++i) {
		for (int j = 0; j < unk2[i].size(); ++j) {
			fp.serialize(unk2[i][j]);
		}
	}
}
