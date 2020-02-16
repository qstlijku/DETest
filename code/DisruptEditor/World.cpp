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
#include <realTreeFile.h>
#include <unordered_set>
#include <Common.h>

World world;

void World::loadWLUAsync() {
	int i = 0;
	Vector<FileInfo> files = FH::getFileList("worlds/" + settings.worldName + "/generated/wlu", settings.wluExtension);
	for (FileInfo& file : files) {
		std::shared_ptr<wluFile> wlu = std::make_shared<wluFile>();
		wlu->shortName = file.name;
		SDL_RWops* fp = FH::openFile(file.fullPath.c_str());
		if (fp) {
			wlu->open(fp);
			SDL_RWclose(fp);

			if (wlu->shortName.find("wlu_data_near") != std::string::npos) {
				wlu->wluType = wlu->WLU_NEAR;
			} else if (wlu->shortName.find("wlu_data_far") != std::string::npos) {
				wlu->wluType = wlu->WLU_FAR;
			} else if (wlu->shortName.find("wlu_data_world") != std::string::npos) {
				wlu->wluType = wlu->WLU_WORLD;
			} else {
				wlu->wluType = wlu->WLU_OTHER;
			}

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

	//Map WLU pos
	for (int32_t x = 0; x < world.GridMapSectorsCount.x / 2; ++x) {
		for (int32_t y = 0; y < world.GridMapSectorsCount.y / 2; ++y) {
			int32_t offset = (y * world.GridMapSectorsCount.x / 2) + x;
			glm::vec2 bbMin((x * world.GridMapSectorsGranularity * 2) - world.GridsWorldOffset.x, (y * world.GridMapSectorsGranularity * 2) - world.GridsWorldOffset.y);
			glm::vec2 bbMax(bbMin + glm::vec2(world.GridMapSectorsGranularity * 2, world.GridMapSectorsGranularity * 2));

			char filename[80];
			snprintf(filename, sizeof(filename), "wlu_data_near%u", offset);
			auto it = world.wlus.find(filename);
			if (it != world.wlus.end()) {
				it->second->bbMin = bbMin;
				it->second->bbMax = bbMax;
				it->second->sectorID = offset;
			}
		}
	}

	for (int32_t x = 0; x < world.GridMapSectorsCount.x / 4; ++x) {
		for (int32_t y = 0; y < world.GridMapSectorsCount.y / 4; ++y) {
			int32_t offset = (y * world.GridMapSectorsCount.x / 4) + x;
			glm::vec2 bbMin((x * world.GridMapSectorsGranularity * 4) - world.GridsWorldOffset.x, (y * world.GridMapSectorsGranularity * 4) - world.GridsWorldOffset.y);
			glm::vec2 bbMax(bbMin + glm::vec2(world.GridMapSectorsGranularity * 4, world.GridMapSectorsGranularity * 4));

			char filename[80];
			snprintf(filename, sizeof(filename), "wlu_data_far%u", offset);
			auto it = world.wlus.find(filename);
			if (it != world.wlus.end()) {
				it->second->bbMin = bbMin;
				it->second->bbMax = bbMax;
				it->second->sectorID = offset;
			}
		}
	}
}

void World::loadSectors() {
	tinyxml2::XMLElement* Grids = world.gameXML->RootElement()->FirstChildElement("Grids");
	world.GridsWorldOffset.x = Grids->IntAttribute("WorldOffsetX");
	world.GridsWorldOffset.y = Grids->IntAttribute("WorldOffsetY");

	tinyxml2::XMLElement* GridMapSectors = Grids->FirstChildElement("GridMapSectors");
	world.GridMapSectorsCount.x = GridMapSectors->IntAttribute("CountX");
	world.GridMapSectorsCount.y = GridMapSectors->IntAttribute("CountY");
	world.GridMapSectorsGranularity = GridMapSectors->IntAttribute("Granularity");

	world.sectors.reserve(world.GridMapSectorsCount.x * world.GridMapSectorsCount.y);
	for (uint32_t x = 0; x < world.GridMapSectorsCount.x; x += 4) {
		for (uint32_t y = 0; y < world.GridMapSectorsCount.y; y += 4) {
			uint32_t offset = (y * world.GridMapSectorsCount.x) + x;
			SDL_Log("Loading Sector %u", offset);

			CSector sector;
			sector.xPos = x;
			sector.yPos = y;
			sector.sectorID = offset;

			char filename[80];
			snprintf(filename, sizeof(filename), "worlds/%s/generated/sdat/sd%u.sdat", settings.worldName.c_str(), offset);
			SDL_RWops* fp = FH::openFile(filename);
			SDL_assert_release(fp);
			if (fp) {
				CBinaryArchiveReader reader(fp);
				sector.open(reader);
				SDL_RWclose(fp);

				world.sectors.push_back(sector);
			}
		}
	}
}

static void setLoadingStatus(const char* str, float progress = 0.f) {
	world.mutex.lock();
	world.loadingStatus = str;
	world.loadingProgress = progress;
	world.mutex.unlock();
}

glm::mat4 convertRotation(const glm::vec3 &a) {
	return glm::eulerAngleYXZ(a.y, a.x, a.z);
}

void World::loaderThread() {
	world.graphicMutex.lock();

	//These two need to be set up before anything else
	setLoadingStatus("Scanning Files");
	FH::Init();

	setLoadingStatus("Setting Up Database");
	DB::instance();

	//Make sure these entries exist for our wlu loader
	/*for (int32_t x = 0; x < world.GridMapSectorsCount.x / 4; ++x) {
		for (int32_t y = 0; y < world.GridMapSectorsCount.y / 4; ++y) {
			int32_t offset = (y * world.GridMapSectorsCount.x) + x;
			char filename[180];
			snprintf(filename, sizeof(filename), "worlds/%s/generated/wlu/wlu_data_near%u.%s", settings.worldName.c_str(), offset, settings.wluExtension.c_str());
			DB::instance().addFNVEntry(filename, filename);
			snprintf(filename, sizeof(filename), "worlds/%s/generated/wlu/wlu_data_far%u.%s", settings.worldName.c_str(), offset, settings.wluExtension.c_str());
			DB::instance().addFNVEntry(filename, filename);
		}
	}*/

	world.gameXML = loadRml("worlds\\" + settings.worldName + "\\generated\\" + settings.worldName + ".game.xml");
	std::string debug = XMLToString(*world.gameXML.get());

	std::future<void> loadEntityLibraryF = std::async(loadEntityLibrary);
	std::future<void> particlesF = std::async([]() { loadRml("worlds/" + settings.worldName + "/generated/" + settings.worldName + "_deploadnewparticles.rml"); });
	std::future<void> loadWLUF = std::async(world.loadWLUAsync);
	std::future<void> loadSectorF = std::async(world.loadSectors);

	world.spawnPointList = loadXmlOrRML("worlds/" + settings.worldName + "/generated/spawnpointlist.xml");

	loadEntityLibraryF.get();
	particlesF.get();
	loadWLUF.get();
	loadSectorF.get();

	world.readyToRender = true;

	world.graphicMutex.unlock();

	world.regenWLUCommandList();
}

void World::onResize() {
}

void World::regenWLUCommandList() {
	//Start Drawing WLUs
	int i = 0;
	for (auto& wlu : world.wlus) {
		wlu.second->gBucket = WorldRenderer::createBucket();

		Node* Entities = wlu.second->root.findFirstChild("Entities");
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
							glm::vec3 pos = entityPtr->getAttrValue<glm::vec3>("hidPos");
							glm::vec3 angles = entityPtr->getAttrValue<glm::vec3>("hidAngles");

							glm::mat4 modelMatrix = glm::translate(glm::mat4(1), pos);
							modelMatrix *= convertRotation(angles);
							
							wlu.second->gBucket->add(path, modelMatrix);
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
							glm::vec3 pos = entityPtr->getAttrValue<glm::vec3>("hidPos");
							glm::vec3 angles = entityPtr->getAttrValue<glm::vec3>("hidAngles");

							glm::mat4 modelMatrix = glm::translate(glm::mat4(1), pos);
							modelMatrix *= convertRotation(angles);

							wlu.second->gBucket->add(path, modelMatrix);
						}
					}
				}
			}

			//Draw Batch
#if 1
			if (entityPtr->getAttrValue<CStringID>("hidEntityClass") == CStringID("CBatchMeshEntity")) {
				Attribute* ExportPath = entityPtr->getAttribute("ExportPath");
				std::string compound = (char*)ExportPath->buffer.data();
				compound = compound.substr(0, compound.size() - strlen(".batch"));
				compound += "_Compound.cbatch";

				std::shared_ptr<batchFile> batch = loadbatchFile(compound);

				if(batch->physicsFile != 0xFFFFFFFF)
					loadCollisionBatchFile(batch->physicsFile);

				auto& component = batch->componentMBP;
				for (auto& it : component.batchProcessors) {
					for (auto& it : it.processors) {
						{
							auto batch = std::get_if<batchFile::CGraphicBatchProcessor>(&it.data);
							if (!batch) continue;

							for (int i = 0; i < batch->data.data.size(); ++i) {
								glm::mat4 mat;
								batch->data.getMatrix(i, mat);

								wlu.second->gBucket->add(batch->xbg.file, mat);
							}
						}

						{
							auto batch = std::get_if<batchFile::CSecurityCameraBatchProcessor>(&it.data);
							if (!batch) continue;

							for (auto &it : batch->objects) {
								if(it)
									wlu.second->gBucket->add(batch->xbg.file, it->unk1);
							}
						}

						{
							auto batch = std::get_if<batchFile::CTrafficLightBatchProcessor>(&it.data);
							if (!batch) continue;

							for (auto& it : batch->trafficLights) {
								if (it)
									wlu.second->gBucket->add(batch->geom.file, it->offset);
							}
						}

						{
							auto batch = std::get_if<batchFile::CRealTreeBatchProcessor>(&it.data);
							if (!batch) continue;

							std::shared_ptr<realTreeFile> model = loadRealTree(batch->resource.file);

						}
					}
				}

				batchFile::CBuildingMultiBatchProcessor& building = batch->buildingMBP;
				for (auto& it : building.buildingResources) {
					std::shared_ptr<buildingBatchFile> buildingBatch = loadBuildingBatchFile(it);

					for (auto& buildingData : buildingBatch->buildingData) {
						for (auto& facade : buildingData.facades) {
							buildingBatchFile::SGfxModelInfo& gfx = buildingBatch->facadeGfxModels.models[facade.unk6];

							for (int i = 0; i < facade.data.data.size(); ++i) {
								glm::mat4 mat;
								facade.data.getMatrix(i, mat);

								wlu.second->gBucket->add(gfx.geomResource.file, mat);
							}
						}
					}
				}
				/*if (building.lowGeom.id != -1) {
					std::shared_ptr<xbgFile> xbg = loadXBG(building.lowGeom.id);

					glm::mat4 modelMatrix = glm::translate(glm::mat4(1), building.unk7);
					RenderInterface::instance().objectCB.Model = modelMatrix;
					xbg->draw(pDeferredContext);
				}*/
				if (building.roofGeom.id != -1) {
					glm::mat4 mat = glm::translate(glm::mat4(1), building.unk3);
					wlu.second->gBucket->add(building.roofGeom, mat);
				}
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

					for (auto& it : loft->networkRegionResources) {
						if (!it) continue;

					}
				}
			}
		}

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
		float xOffset = sector.xPos * GridMapSectorsGranularity;
		float yOffset = sector.yPos * GridMapSectorsGranularity;

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

				RenderInterface::instance().objectCB.Model = glm::translate(glm::mat4(1), glm::vec3((xOffset + x * 64) - GridsWorldOffset.x, (yOffset + y * GridMapSectorsGranularity) - GridsWorldOffset.y, 0));
				RenderInterface::instance().objectCB.Offset = glm::vec4(x / 4.f, y / 4.f, 0, 0);
				context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

				UINT offset = 0;
				context->IASetVertexBuffers(0, 1, &vertexBuffer->pVertexBuffer, &vertexBuffer->stride, &offset);
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				context->IASetInputLayout(m_inputLayout.Get());
				context->DrawIndexed(CSectorHighRes::CSceneTerrainSectorPackedData::getIndexBuffer()->size, 0, 0);
			}
		}
	}
}
