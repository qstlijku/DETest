#include <SDL.h>
#include "Colors.h"
#include "Common.h"
#include "DDRenderInterface.h"
#include "cseqFile.h"
#include "wluFile.h"
#include "xbgFile.h"
#include "materialFile.h"
#include "Camera.h"
#include <map>
#include <unordered_map>
#include "Hash.h"
#include "imgui.h"
#include "LoadingScreen.h"
#include "Dialog.h"
#include "Entity.h"
#include "DominoBox.h"
#include <future>
#include <unordered_set>
#include <Ntsecapi.h>
#include "RML.h"
#include "glm/gtc/matrix_transform.hpp"
#include "CSector.h"
#include "batchFile.h"
#include "Audio.h"
#include "ImGuizmo.h"
#include "FileHandler.h"
#include "World.h"
#include "ImguiWindows.h"
#include "Serialization.h"
#include "Version.h"
#include "IBinaryArchive.h"
#include "HexBase64.h"
#include "DB.h"
#include "DARE.h"
#include "SplineLoft.h"
#include "CResourceDataBase.h"
#include "embedFile.h"
#include "CMoveResourceDataManager.h"
#include "batchFile.h"
#include "ResourceLoader.h"
#include <glm/gtx/norm.hpp>
#include "WaterMeshes.h"
#include "CLODictionary.h"
#include <filesystem>

#include <Windows.h>
#include <Shellapi.h>
#include <DbgHelp.h>
#include <RoadNetwork.h>
#include <SDL_syswm.h>
#include <imgui_impl_sdl.h>
static LONG WINAPI HandleException(struct _EXCEPTION_POINTERS* apExceptionInfo) {
	HANDLE hFile = ::CreateFile(L"crash.mdmp", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile) {
		_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = apExceptionInfo;
		ExInfo.ClientPointers = FALSE;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		::CloseHandle(hFile);

		MessageBox(NULL, L"DisruptEditor has crashed! A crash.mdmp has been written",
			L"Your session has been Disrupted!",
			MB_ICONERROR | MB_OK
		);
	}
	exit(0);
}

static void LogOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message) {
	FILE* fp = (FILE*)userdata;
	fprintf(fp, "%s\n", message);
	fflush(fp);
}

int main(int argc, char **argv) {
	SetUnhandledExceptionFilter(HandleException);
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_LogSetOutputFunction(LogOutputFunction, fopen("DisruptEditor.log", "wb"));

	LoadingScreen *loadingScreen = new LoadingScreen;

	reloadSettings();

	SDL_Window *window = RenderInterface::instance().window;

	Camera &camera = RenderInterface::instance().camera;
	camera.type = Camera::FLYCAM;

#if _DEBUG
	{
		/*{
			//12 byte header?
			//then total size
			SDL_RWops* fp = FH::openFile("worlds/windy_city/generated/citylifedatadict.dat");
			CBinaryArchiveReader reader(fp);
			SDL_RWseek(fp, 36, RW_SEEK_SET);
			CLODataDictionaries dict;
			dict.read(reader);
			SDL_RWclose(fp);
		}

		{
			CCityLifeDataAndStateHandler state;
			Vector<uint8_t> data = fromHexString("00000084000000900000668000000000000000840000000000000084000000000000000010929A090000000300000000000000020000000000000000000000000000002F00000000000000000000000200000000000000000000000100000000000000000000000000000000000000000000000100000000000000000000000000000148000000000000000000000000");
			SDL_RWops* fp = SDL_RWFromConstMem(data.data(), data.size());
			SDL_RWseek(fp, 36, RW_SEEK_SET);
			CBinaryArchiveReader reader(fp);
			reader.bigEndian = true;
			state.read(reader);
		}*/

		/*{
			CMoveResourceDataManager move;
			SDL_RWops* fp = FH::openFile("worlds/windy_city/generated/combinedmovefile.bin");
			CBinaryArchiveReader reader(fp);
			move.open(reader);
		}

		{
			WaterMeshes db;
			SDL_RWops* fp = FH::openFile("worlds/windy_city/generated/watermeshes.fcb");
			CBinaryArchiveReader reader(fp);
			db.open(reader);
			SDL_RWclose(fp);

			db.indexes.clear();
			db.meshes.clear();
			fp = FH::openFileWrite("worlds/windy_city/generated/watermeshes.fcb");
			CBinaryArchiveWriter writer(fp);
			db.open(writer);
			SDL_RWclose(fp);
		}

		{
			CResourceDataBase db;
			SDL_RWops* fp = FH::openFile("worlds/windy_city/generated/windy_city_depload.dat");
			CBinaryArchiveReader reader(fp);
			db.open(reader);
			writeFile("db.xml", serializeToXML(db));
		}*/


		/*{
			RoadNetwork move;
			SDL_RWops* fp = FH::openFile("worlds/windy_city/generated/roadnetwork/roadnetwork_lowres.rnf");
			CBinaryArchiveReader reader(fp);
			move.read(reader);
		}*/
	}
#endif

	{
		//These two need to be set up before anything else
		std::future<void> fileHandlerF = std::async(FH::Init);
		std::future<void> dbF = std::async([]() { DB::instance(); });

		loadingScreen->setTitle("Scanning Files");
		loadingScreen->waitForFuture(fileHandlerF);
		loadingScreen->setTitle("Setting Up Database");
		loadingScreen->waitForFuture(dbF);

		std::future<void> loadEntityLibraryF = std::async(loadEntityLibrary);
		std::future<void> particlesF = std::async([]() { loadRml(FH::openFile("worlds/windy_city/generated/windy_city_deploadnewparticles.rml")); });
		std::future<void> loadWLUF = std::async(world.loadWLUAsync);
		std::future<void> loadSectorF = std::async(world.loadSectors);

		std::thread resourceLoader(resourceLoaderThread);
		resourceLoader.detach();

		world.spawnPointList = loadXml(FH::openFile("worlds/windy_city/generated/spawnpointlist.xml"));

		loadingScreen->setTitle("Loading Entity Library");
		loadingScreen->waitForFuture(loadEntityLibraryF);
		loadingScreen->setTitle("Loading Particle Library");
		loadingScreen->waitForFuture(particlesF);
		loadingScreen->setTitle("Loading World Load Units");
		loadingScreen->waitForFuture(loadWLUF);
		loadingScreen->setTitle("Loading World Sectors");
		loadingScreen->waitForFuture(loadSectorF);

		//Unused for now
		//loadingScreen->setTitle("Loading Language Files");
		//Dialog::instance();
	}

	Uint32 ticks = SDL_GetTicks();
	uint64_t frameCount = 0;

	if (settings.maximized)
		SDL_MaximizeWindow(window);
	SDL_ShowWindow(window);
	delete loadingScreen;
	loadingScreen = NULL;

	bool windowOpen = true;
	while (windowOpen) {
		float delta = (SDL_GetTicks() - ticks) / 1000.f;
		ticks = SDL_GetTicks();
		if (delta > 0.5f)
			delta = 0.5f;

		RenderInterface &renderInterface = RenderInterface::instance();

		glm::ivec2 windowSize;
		SDL_GetWindowSize(window, &windowSize.x, &windowSize.y);
		renderInterface.sceneCB.windowSize = windowSize;
		renderInterface.sceneCB.View = glm::lookAtLH(camera.location, camera.lookingAt, camera.up);
		renderInterface.sceneCB.Projection = glm::perspective(settings.fov, (float)windowSize.x / windowSize.y, settings.near_plane, settings.far_plane);
		renderInterface.sceneCB.ViewProjection = renderInterface.sceneCB.Projection * renderInterface.sceneCB.View;
		RenderInterface::instance().newFrame();

		world.mutex.lock();

		UI::displayTopMenu();
		UI::displayTempWindows();
		UI::displayWindows();

		int resources;
		std::string file;
		getResourceLoaderProgress(resources, file);
		if (world.loadingProgress != 1.f || resources > 0 || true) {
			ImGui::SetNextWindowPos(ImVec2(15, 25), ImGuiCond_Always);
			ImGui::Begin("Loading", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("%i%% %s", (int)(world.loadingProgress * 100), world.loadingStatus.c_str());
			ImGui::Text("Resource Loader: %i %s", resources, file.c_str());
			ImGui::End();
		}
		
		dd::xzSquareGrid(-50, 50, 0, 1, blue);

		/*if (settings.drawTerrain) {
			RenderInterface::instance().terrain.use();
			size_t maxSectors = world.sectors.size();
			for (size_t i = 0; i < maxSectors; ++i)
				world.sectors[i].draw();
		}*/

		//Draw XBG
		/*glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		RenderInterface::instance().model.use();
		glm::mat4 MVP = RenderInterface::instance().VP;
		glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
		CHECK_GL_ERROR();
		static std::shared_ptr<xbgFile> xbg;
		static int selLod = 0;
		static char buffer[255];
		ImGui::InputText("Filename", buffer, sizeof(buffer));
		if (ImGui::Button("Load")) {
			xbg = loadXBG(buffer);
			selLod = 0;
		}
		static uint32_t hashID = 0;
		ImGui::InputScalar("HashID", ImGuiDataType_U32, &hashID);
		if (ImGui::Button("Load ID")) {
			xbg = loadXBG(hashID);
			selLod = 0;
		}
		if (ImGui::Button("XML")) {
			std::string str = serializeToXML(*xbg);
			SDL_SetClipboardText(str.c_str());
		}
		if (xbg) {
			ImGui::SliderInt("Lod", &selLod, 0, xbg->lods.size() - 1);
			xbg->draw(selLod);
		}*/

		//Draw Building Batches
		/*for (auto& it : world.batches) {
			auto& component = it.second->componentMBP;
			for (auto& it : component.batchProcessors) {
				for (auto& it : it.processors) {
					auto batch = std::get_if<batchFile::CGraphicBatchProcessor>(&it.data);
					if (!batch) continue;

					std::shared_ptr<xbgFile> xbg = loadXBG(batch->xbg.file.id);
					for (auto& it : batch->ranges) {
						if (glm::distance2(RenderInterface::instance().camera.location, it.unk1) > 100 * 100)
							continue;
						glm::mat4 MVP = glm::translate(RenderInterface::instance().VP, it.unk1);
						glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
						xbg->draw(0);
					}
				}
			}

			batchFile::CBuildingMultiBatchProcessor& building = it.second->buildingMBP;
			if (building.lowGeom.id != -1) {
				std::shared_ptr<xbgFile> xbg = loadXBG(building.lowGeom.id);
				glm::mat4 MVP = glm::translate(RenderInterface::instance().VP, building.unk6);
				glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
				xbg->draw(0);
			}
			if (building.roofGeom.id != -1) {
				std::shared_ptr<xbgFile> xbg = loadXBG(building.roofGeom.id);
				glm::mat4 MVP = glm::translate(RenderInterface::instance().VP, building.unk6);
				glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
				xbg->draw(0);
			}
			
			/*if (!building.buildingResources.empty())
				std::string a = serializeToXML(*it.second);*//*
		}*/

		ImGui::InputFloat3("CameraPos", &RenderInterface::instance().camera.location.x);

		world.mutex.unlock();

		if (!ImGui::IsAnyWindowHovered())
			camera.update(delta);

		RenderInterface::instance().endFrame();
		frameCount++;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type) {
			case SDL_WINDOWEVENT: {
				switch(event.window.event) {
				case SDL_WINDOWEVENT_CLOSE:
					windowOpen = false;
					saveSettings();
					exit(0);
				case SDL_WINDOWEVENT_RESIZED:
					settings.windowSize = glm::ivec2(event.window.data1, event.window.data2);
					saveSettings();
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					settings.maximized = true;
					saveSettings();
					break;
				case SDL_WINDOWEVENT_RESTORED:
					settings.maximized = false;
					saveSettings();
					break;
				}
				break;
			}
			case SDL_DROPFILE: {
				SDL_RWops* fp = SDL_RWFromFile(event.drop.file, "rb");
				Node root = readFCB(fp);
				tinyxml2::XMLPrinter printer;
				root.serializeXML(printer);
				std::string output = event.drop.file + std::string(".xml");
				SDL_RWclose(fp);
				fp = SDL_RWFromFile(output.c_str(), "wb");
				SDL_RWwrite(fp, printer.CStr(), printer.CStrSize(), 1);
				SDL_RWclose(fp);
				break;
			}
			}
		}
	}

	ImGui::DestroyContext();
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}