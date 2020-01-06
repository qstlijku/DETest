#include "World.h"

#include "FileHandler.h"
#include "IBinaryArchive.h"
#include <SDL.h>
#include <future>
#include <DB.h>
#include <RML.h>
#include "Entity.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <Hash.h>
#include <ResourceLoader.h>
#include <xbgFile.h>
#include <SplineLoft.h>
#include <glm\gtx\quaternion.hpp>
#include <buildingBatchFile.h>

World world;

void World::loadWLUAsync() {
	int i = 0;
	Vector<FileInfo> files = FH::getFileList("worlds/windy_city/generated/wlu", "xml.data.fcb");
	for (FileInfo& file : files) {
		std::shared_ptr<wluFile> wlu = std::make_shared<wluFile>();
		wlu->shortName = file.name;
		SDL_RWops* fp = FH::openFile(file.fullPath.c_str());
		if (fp) {
			wlu->open(fp);
			SDL_RWclose(fp);

			world.mutex.lock();
			world.wlus[file.name] = wlu;
			++i;
			world.loadingStatus = file.name;
			world.loadingProgress = (float)i / files.size();
			world.mutex.unlock();
		} else {
			SDL_Log("Failed to open %s", file.fullPath.c_str());
		}
	}
}

void World::loadSectors() {
	//This is hardcoded!
	world.sectors.reserve(64 * 80);
	for (uint32_t x = 0; x < 64; x += 4) {
		for (uint32_t y = 0; y < 80; y += 4) {
			uint32_t offset = (y * 64) + x;
			SDL_Log("Loading Sector %u", offset);

			CSector sector;
			sector.xPos = x;
			sector.yPos = y;
			sector.sectorID = offset;

			char filename[80];
			snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdat", offset);
			SDL_RWops* fp = FH::openFile(filename);
			SDL_assert_release(fp);
			CBinaryArchiveReader reader(fp);
			sector.open(reader);
			SDL_RWclose(fp);

			world.sectors.push_back(sector);
		}
	}
}

static void setLoadingStatus(const char* str, float progress = 0.f) {
	world.mutex.lock();
	world.loadingStatus = str;
	world.loadingProgress = progress;
	world.mutex.unlock();
}

glm::mat4 convertRotation(const glm::vec3 a) {
	float sx = sin(a.x / 2), sy = sin(a.y / 2), sz = sin(a.z / 2);
	float cx = cos(a.x / 2), cy = cos(a.y / 2), cz = cos(a.z / 2);

	glm::quat b(cx * cy * cz + sx * sy * sz,
		sx * cy * cz - cx * sy * sz,
		cx * sy * cz + sx * cy * sz,
		cx * cy * sz - sx * sy * cz); // for XYZ application order
	return glm::toMat4(b);
}

void World::loaderThread() {
	//These two need to be set up before anything else
	setLoadingStatus("Scanning Files");
	FH::Init();

	setLoadingStatus("Setting Up Database");
	DB::instance();

	std::future<void> loadEntityLibraryF = std::async(loadEntityLibrary);
	std::future<void> particlesF = std::async([]() { loadRml(FH::openFile("worlds/windy_city/generated/windy_city_deploadnewparticles.rml")); });
	std::future<void> loadWLUF = std::async(world.loadWLUAsync);
	std::future<void> loadSectorF = std::async(world.loadSectors);

	world.spawnPointList = loadXml(FH::openFile("worlds/windy_city/generated/spawnpointlist.xml"));

	loadEntityLibraryF.get();
	particlesF.get();
	loadWLUF.get();
	loadSectorF.get();

	//Create a deffered context
	HRESULT hr;
	ID3D11DeviceContext* pDeferredContext = NULL;
	hr = RenderInterface::instance().g_pd3dDevice->CreateDeferredContext(0, &pDeferredContext);
	assert(hr == S_OK);
	RenderInterface::instance().setupState(pDeferredContext);

	//Render Terrain
	world.loadingStatus = "Preloading Sectors";
	world.drawTerrain(pDeferredContext);

	//Start Creating command queues
	hr = pDeferredContext->FinishCommandList(FALSE, &world.pd3dCommandList);
	assert(hr == S_OK);

	world.readyToRender = true;

	//Start Drawing WLUs
	int i = 0;
	for (auto& it : world.wlus) {
		RenderInterface::instance().setupState(pDeferredContext);

		pDeferredContext->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
		pDeferredContext->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);

		Node* Entities = it.second->root.findFirstChild("Entities");
		for (Node& entityRef : Entities->children) {
			Node* entityPtr = &entityRef;
			Node* archeType = entityPtr;
			Attribute* ArchetypeGuid = entityRef.getAttribute("ArchetypeGuid");
			if (ArchetypeGuid) {
				uint32_t uid = Hash::getFilenameHash((const char*)ArchetypeGuid->buffer.data());
				archeType = findEntityByUID(uid);
				if (!archeType) {
					SDL_Log("Could not find %s\n", ArchetypeGuid->buffer.data());
					archeType = entityPtr;
				}
			}

			//Add to AABB
			{
				glm::vec3& pos = entityPtr->get<glm::vec3>("hidPos");
				it.second->aabb.minp.x = glm::min(it.second->aabb.minp.x, pos.x);
				it.second->aabb.minp.y = glm::min(it.second->aabb.minp.y, pos.y);
				it.second->aabb.minp.z = glm::min(it.second->aabb.minp.z, pos.z);
				it.second->aabb.maxp.x = glm::max(it.second->aabb.maxp.x, pos.x);
				it.second->aabb.maxp.y = glm::max(it.second->aabb.maxp.y, pos.y);
				it.second->aabb.maxp.z = glm::max(it.second->aabb.maxp.z, pos.z);
			}

			Node* Components = entityPtr->findFirstChild("Components");
			Node* archeComponents = archeType->findFirstChild("Components");
			if (!Components) continue;

			//Draw XBG
			{
				Node* CGraphicComponent = Components->findFirstChild("CGraphicComponent");
				Node* archeCGraphicComponent = archeComponents->findFirstChild("CGraphicComponent");
				if (CGraphicComponent || archeCGraphicComponent) {
					Attribute* fileModel = NULL;
					if(archeCGraphicComponent && archeCGraphicComponent->getAttribute("fileModel"))
						fileModel = archeCGraphicComponent->getAttribute("fileModel");
					if (CGraphicComponent && CGraphicComponent->getAttribute("fileModel"))
						fileModel = CGraphicComponent->getAttribute("fileModel");
					if (fileModel) {
						CPathID path;
						memcpy(&path, fileModel->buffer.data(), sizeof(CPathID));

						if (path.id != 0xffffffff) {
							auto model = loadXBG(path);

							glm::vec3& pos = entityPtr->get<glm::vec3>("hidPos");
							glm::vec3& angles = entityPtr->get<glm::vec3>("hidAngles");

							glm::mat4 modelMatrix = glm::translate(glm::mat4(1), pos);
							modelMatrix *= convertRotation(angles);
							RenderInterface::instance().objectCB.Model = modelMatrix;

							model->draw(pDeferredContext);
						}
					}
				}
			}

			{
				Node* CGraphicComponent = Components->findFirstChild("CSkinnedGraphicComponent");
				Node* archeCGraphicComponent = archeComponents->findFirstChild("CSkinnedGraphicComponent");
				if (CGraphicComponent || archeCGraphicComponent) {
					Attribute* fileModel = NULL;
					if (archeCGraphicComponent && archeCGraphicComponent->getAttribute("fileModel"))
						fileModel = archeCGraphicComponent->getAttribute("fileModel");
					if (CGraphicComponent && CGraphicComponent->getAttribute("fileModel"))
						fileModel = CGraphicComponent->getAttribute("fileModel");
					if (fileModel) {
						CPathID path;
						memcpy(&path, fileModel->buffer.data(), sizeof(CPathID));

						if (path.id != 0xffffffff) {
							auto model = loadXBG(path);

							glm::vec3& pos = entityPtr->get<glm::vec3>("hidPos");
							glm::vec3& angles = entityPtr->get<glm::vec3>("hidAngles");

							glm::mat4 modelMatrix = glm::translate(glm::mat4(1), pos);
							modelMatrix *= convertRotation(angles);
							RenderInterface::instance().objectCB.Model = modelMatrix;

							model->draw(pDeferredContext);
						}
					}
				}
			}

			//Draw Batch
#if 0
			if (entityPtr->get<CStringID>("hidEntityClass") == CStringID("CBatchMeshEntity")) {
				Attribute* ExportPath = entityPtr->getAttribute("ExportPath");
				std::string compound = (char*)ExportPath->buffer.data();
				compound = compound.substr(0, compound.size() - strlen(".batch"));
				compound += "_Compound.cbatch";

				std::shared_ptr<batchFile> batch = loadbatchFile(compound);
				/*auto& component = batch->componentMBP;
				for (auto& it : component.batchProcessors) {
					for (auto& it : it.processors) {
						auto batch = std::get_if<batchFile::CGraphicBatchProcessor>(&it.data);
						if (!batch) continue;

						std::shared_ptr<xbgFile> model = loadXBG(batch->xbg.file);
						for (auto& it : batch->ranges) {
							glm::mat4 modelMatrix = glm::translate(glm::mat4(1), it.unk3);

							glm::vec3& angles = it.unk9;
							modelMatrix *= convertRotation(angles);

							RenderInterface::instance().objectCB.Model = modelMatrix;

							model->draw(pDeferredContext);
						}
					}
				}*/

				batchFile::CBuildingMultiBatchProcessor& building = batch->buildingMBP;
				for (auto& it : building.buildingResources) {
					//std::shared_ptr<buildingBatchFile> buildingBatch = loadBuildingBatchFile(it);


				}
				/*if (building.lowGeom.id != -1) {
					std::shared_ptr<xbgFile> xbg = loadXBG(building.lowGeom.id);

					glm::mat4 modelMatrix = glm::translate(glm::mat4(1), building.unk7);
					RenderInterface::instance().objectCB.Model = modelMatrix;
					xbg->draw(pDeferredContext);
				}
				if (building.roofGeom.id != -1) {
					std::shared_ptr<xbgFile> xbg = loadXBG(building.roofGeom.id);

					glm::mat4 modelMatrix = glm::translate(glm::mat4(1), building.unk7);
					RenderInterface::instance().objectCB.Model = modelMatrix;

					xbg->draw(pDeferredContext);
				}*/
			}
#endif

			//Draw HiResSplineLoft
			{
				Node* CSplineLoftHiResGFXComponent = Components->findFirstChild("CSplineLoftHiResGFXComponent");
				if (CSplineLoftHiResGFXComponent) {
					Attribute* ResourcePathID = CSplineLoftHiResGFXComponent->getAttribute("ResourcePathID");
					CPathID path;
					memcpy(&path, ResourcePathID->buffer.data(), sizeof(CPathID));

					std::shared_ptr<SplineLoftHiRes> loft = loadHiResSplineLoft(path);
					loft->draw(pDeferredContext);
				}
			}
		}

		hr = pDeferredContext->FinishCommandList(FALSE, &it.second->plist);
		assert(hr == S_OK);

		world.loadingProgress = (i++ + world.sectors.size() + 1) / ((float)world.wlus.size() + world.sectors.size());
	}
}

void World::drawTerrain(ID3D11DeviceContext* context) {
	//Create Terrain Input Layout
	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	if (!m_inputLayout)
		RenderInterface::instance().g_pd3dDevice->CreateInputLayout(
			vertexDesc,
			ARRAYSIZE(vertexDesc),
			RenderInterface::instance().terrain.vShaderBlob->GetBufferPointer(),
			RenderInterface::instance().terrain.vShaderBlob->GetBufferSize(),
			&m_inputLayout);

	context->VSSetShader(RenderInterface::instance().terrain.pVertexShader, NULL, NULL);
	context->PSSetShader(RenderInterface::instance().terrain.pPixelShader, NULL, NULL);
	context->IASetIndexBuffer(CSectorHighRes::CSceneTerrainSectorPackedData::getIndexBuffer()->pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	ID3D11SamplerState* samplers[] = {
			RenderInterface::instance().tex0,
			RenderInterface::instance().tex0,
			RenderInterface::instance().tex0,
	};
	context->PSSetSamplers(0, 3, samplers);

	size_t maxSectors = world.sectors.size();
	for (size_t i = 0; i < maxSectors; ++i) {
		auto& sector = world.sectors[i];
		std::shared_ptr<CSectorHighRes> hiRes = sector.getHiRes();
		float xOffset = sector.xPos * 64;
		float yOffset = sector.yPos * 64;

		ID3D11ShaderResourceView* views[] = {
			sector.getColorTexture()->pResource,
			sector.getDiffuseTexture()->pResource,
			sector.getMaskTexture()->pResource,
		};
		context->PSSetShaderResources(0, 3, views);

		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				auto& map = hiRes->getMap(x, y);
				auto vertexBuffer = map.getVertexBuffer();

				RenderInterface::instance().objectCB.Model = glm::translate(glm::mat4(), glm::vec3((xOffset + x * 64) - 2048 - 32, (yOffset + y * 64) - 2560 - 32, 0));
				RenderInterface::instance().objectCB.Offset = glm::vec4(x / 4.f, y / 4.f, 0, 0);
				context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

				UINT offset = 0;
				context->IASetVertexBuffers(0, 1, &vertexBuffer->pVertexBuffer, &vertexBuffer->stride, &offset);
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				context->IASetInputLayout(m_inputLayout.Get());
				context->DrawIndexed(CSectorHighRes::CSceneTerrainSectorPackedData::getIndexBuffer()->size, 0, 0);
			}
		}

		world.loadingProgress = (i + 1) / ((float)world.wlus.size() + world.sectors.size());
	}
}
