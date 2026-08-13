#include "SplineLoft.h"
#include "ResourceLoader.h"
#include "materialFile.h"
#include "xbtFile.h"

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
	lowResPath = lowRes.getReverseFilename();
	fp.finish();

	SDL_assert_release(vertexData.size() != 1603);
	SDL_assert_release(indexData.size() != 14355);
}

// rangeDescs, size 7 (could loop thru)
// primitives, size 47 (not much here powers of 2 etc)
// primitiveDrawCallRanges, size 58 (try looping thru here)
void SplineLoftLowRes::draw(ID3D11DeviceContext* context) {
	for (auto& rangeDesc : unk3.unk13) {
		//RenderInterface::instance().objectCB.Offset = glm::vec4(geomParams.unk1, geomParams.unk2, geomParams.unk4, geomParams.unk5);

		D3D_PRIMITIVE_TOPOLOGY pType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//D3D_PRIMITIVE_TOPOLOGY pType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		context->IASetPrimitiveTopology(pType);

		// TODO: Load other textures like we do for xbg files
		uint8_t matIdx = rangeDesc.unk3;
		auto materialFile = materials[matIdx];
		auto material = loadMaterial(materialFile);
		auto paths = material->getTexturePaths();
		int numViews = paths.size();
		auto diffuseFile = material->getCommandPath("DiffuseTexture1");
		auto diffuse = loadTexture(diffuseFile.c_str());
		context->PSSetShaderResources(0, 1, &diffuse->pResource);

		UINT offset = 0; // TODO: what is the correct offset?
		UINT stride = 32; // TODO: is this serialized anywhere?
		context->IASetVertexBuffers(0, 1, &vertex->pVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(index->pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		static Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
		if (!layout.Get()) {
			const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
			  { "POSITION", 0, DXGI_FORMAT_R16G16B16A16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			  { "TEXCOORD", 0, DXGI_FORMAT_R16G16B16A16_SINT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
		context->DrawIndexed(rangeDesc.unk2 - rangeDesc.unk1, rangeDesc.unk1, 0);
		//context->DrawIndexed(indexData.size(), 0, 0);
	}
}

void SplineLoftHiRes::draw(ID3D11DeviceContext* context) {
	// TODO: Investigate CSplineLoftRenderer
	// and CSplineLoftDrawCall
	// Road res has vertexData and indexData, plaza res does not
	if (vertexData.size() == 0 || indexData.size() == 0)
	{
		// Try loading low res
        //auto loResLoft = loadLowResSplineLoft(lowRes);
        //loResLoft->draw(context);
		return;
	}
	RenderInterface::instance().objectCB.Offset = glm::vec4(1, 0.5, 0.5, 0);
	RenderInterface::instance().objectCB.Model = glm::mat4(1);
	context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

	context->VSSetShader(RenderInterface::instance().spline.pVertexShader, NULL, NULL);
	context->PSSetShader(RenderInterface::instance().spline.pPixelShader, NULL, NULL);

	// TODO set primitive type D3D_PRIMITIVE_TOPOLOGY_POINTLIST
	D3D_PRIMITIVE_TOPOLOGY pType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
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
		  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	for (auto& netRes : networkRegionResources) {
		for (auto& spline : netRes->loftRegion.splineSubsets)
		{
			if (spline == nullptr)
				continue;
			for (CSplineControlPoint cp : spline->controlPoints)
			{
				dd::sphere(&cp.position.x, magenta, 2);
			}
		}
	}

	//context->Draw(vertexData.size(), 0);
	context->DrawIndexed(indexData.size(), 0, 0);
}

void SplineLoftHiRes::createBuffers() {
	if (vertexData.size() == 0 || indexData.size() == 0)
		return;

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
	/*
	fp.serializeConstant<uint32_t>(1280263764);
	fp.serializeConstant<uint32_t>(24);

	fp.serialize(unk1);
	fp.serializeConstant<uint32_t>(0);
	fp.serializeConstant<uint32_t>(0);
	fp.serialize(size);//-12 bytes
	fp.serializeConstant<uint32_t>(0);
	fp.serialize(count);

	if (count == 0)
		return;

	for (uint32_t i = 0; i < count; ++i) {

	}*/
}

void CSceneSplineLoftRegion::read(IBinaryArchive& fp) {
	fp.serialize(regionOffset);
	fp.serialize(regionSize);
	fp.serialize(regionID);
	fp.serializeNdVectorExternal(splineSubsets, "CSpline");

	fp.serialize(splunk4);
	fp.PreAllocateSizeOfType(0xA1A47E4E, splunk4);//Something Like CSceneObjectHandle<CSceneMaterial> *

	fp.serialize(splunk5);
	fp.PreAllocateSizeOfType("CSplineLoftElementDBInfo", splunk5);

	static_assert(sizeof(SSplineRangeCoords) == (1 << 4));
	fp.serializeNdVectorInPlace(rangeCoords, 4);

	static_assert(sizeof(CRangeOffsets) == 0x24);
	fp.serializeNdVectorInPlace(rangeOffsets, 4);

	fp.serializeNdVectorExternal(morphing, "CSplineLoftMorphing");

	static_assert(sizeof(CSplineLoftMeshDesc) == 0x8);
	fp.serializeNdVectorInPlace(meshDescs, 4);

	static_assert(sizeof(CSplineLoftMeshLODGFXDesc) == 2);
	fp.serializeNdVectorInPlace(meshLODGFXDescs, 4);

	fp.serializeNdVectorInPlace(lodDistances, 4);

	fp.serializeNdVectorInPlace(lods, 8);

	static_assert(sizeof(CSplineLoftPrimitiveDrawCallDesc) == 4);
	fp.serializeNdVectorInPlace(primitiveDrawCallDesc, 4);

	static_assert(sizeof(CSplineLoftPrimitiveDrawCallGFXBuffers) == 0x10);
	fp.serializeNdVectorInPlace(primitiveDrawCallGFXBufDesc, 4);
}

void CSplineNetworkRegionResourceEntry::read(IBinaryArchive& fp) {
	fp.PreAllocateSizeOfType("CSceneSplineLoftRegion", 1);

	fp.serialize(loftRegion);
	fp.serializeNdVectorExternal(primitiveDesc, "CSplineLoftPrimitiveDesc");
	fp.serializeNdVectorExternal(drawCalls, "SSplineLoftDrawCall");
	SDL_assert_release(drawCalls.size() == 0);
	fp.serializeNdVector(materials);
	fp.serializeNdVectorInPlace(elementDBIDs, 4);

	for (int i = 0; i < materials.size(); i++)
	{
		materialPaths.push_back(materials[i].getReverseFilename());
	}
}

void CSpline::read(IBinaryArchive& fp) {
	fp.serialize(splineName);
	fp.serialize(id);
	fp.serialize(isLinear);
	fp.serialize(needsExport);

	fp.serializeNdVectorInPlace(controlPoints, 0x10);
}

void CSplineControlPoint::read(IBinaryArchive& fp) {
	fp.serialize(position);
	fp.serialize(rotation);
	fp.serialize(length);
	fp.serialize(cpIdx);
	fp.serialize(tangentIn);
	fp.serialize(tangentOut);
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
	fp.serialize(count);
	materials.resize(count);
	fp.PreAllocateMemory(count << 2, 4);
	for (auto& it : materials)
		fp.serialize(it);

	for (int i = 0; i < materials.size(); i++)
	{
		materialPaths.push_back(materials[i].getReverseFilename());
	}

	fp.PreAllocateSizeOfType(0xA1A47E4E, materials.size());
	fp.serialize(unk3);
	fp.serialize(hiRes);

	uint32_t vertexDataSize = vertexData.size();
	uint32_t indexDataSize = indexData.size();

	fp.serialize(vertexDataSize);
	fp.serialize(indexDataSize);

	fp.finish();

	vertexData.resize(vertexDataSize);
	fp.memBlock(vertexData.data(), 1, vertexData.size());

	indexData.resize(indexDataSize);
	fp.memBlock(indexData.data(), 1, indexData.size());

	createBuffers();
}

void SplineLoftLowRes::createBuffers() {
	if (vertexData.size() == 0 || indexData.size() == 0)
		return;

	vertex = std::make_shared<VertexBuffer>();
	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = vertexData.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(vertexData.size(), D3D11_BIND_VERTEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertex->pVertexBuffer);

	index = std::make_shared<IndexBuffer>();
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indexData.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(indexData.size(), D3D11_BIND_INDEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&index->pIndexBuffer);
	index->size = indexData.size();
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
	fp.serialize(start);
	fp.serialize(count);
}

void CSceneSplineLoftBatch::SPrimitiveData::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	for (int i = 0; i < unk2.size(); ++i) {
		for (int j = 0; j < unk2[i].size(); ++j) {
			fp.serialize(unk2[i][j]);
		}
	}
}
