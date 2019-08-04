#include "World.h"

#include "FileHandler.h"
#include "IBinaryArchive.h"
#include <SDL.h>
#include <future>
#include <DB.h>
#include <RML.h>
#include "Entity.h"
#include <glm\gtc\matrix_transform.hpp>

World world;

void World::loadWLUAsync() {
	Vector<FileInfo> files = FH::getFileList("worlds/windy_city/generated/wlu", "xml.data.fcb");
	int i = 0;
	for (FileInfo& file : files) {
		
		std::shared_ptr<wluFile> wlu = std::make_shared<wluFile>();
		wlu->shortName = file.name;
		SDL_RWops* fp = FH::openFile(file.fullPath.c_str());
		wlu->open(fp);
		SDL_RWclose(fp);
		world.mutex.lock();
		world.wlus[file.name] = wlu;
		world.loadingStatus = file.name;
		++i;
		world.loadingProgress = (float)i / files.size();
		world.mutex.unlock();
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

void World::loaderThread() {
	//These two need to be set up before anything else
	std::future<void> fileHandlerF = std::async(FH::Init);
	std::future<void> dbF = std::async([]() { DB::instance(); });

	setLoadingStatus("Scanning Unknown Files");
	fileHandlerF.get();
	setLoadingStatus("Setting Up Database");
	dbF.get();

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

				RenderInterface::instance().objectCB.Model = glm::translate(glm::mat4(), glm::vec3((xOffset + x * 64) - 2048, (yOffset + y * 64) - 2560, 0));
				RenderInterface::instance().objectCB.UVOffset = glm::vec2(x / 4.f, y / 4.f);
				context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

				UINT offset = 0;
				context->IASetVertexBuffers(0, 1, &vertexBuffer->pVertexBuffer, &vertexBuffer->stride, &offset);
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				context->IASetInputLayout(m_inputLayout.Get());
				context->DrawIndexed(CSectorHighRes::CSceneTerrainSectorPackedData::getIndexBuffer()->size, 0, 0);
			}
		}

		world.loadingProgress = (i + 1) / (float)maxSectors;
	}
}
