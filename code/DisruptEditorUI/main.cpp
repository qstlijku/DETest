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
#include <dr_wav.h>
#include <buildingBatchFile.h>
#include <DB.h>
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

class Test {
public:
	Test() {
		STARTUPINFOW info;
		GetStartupInfoW(&info);

		__debugbreak();
	}
};

Test test;

void renderProgressBar() {
	if (world.loadingProgress >= 1.f)
		return;

	if (world.readyToRender)
		ImGui::SetNextWindowPos(ImVec2(15, 25), ImGuiCond_Always);
	else
		ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(400, 78), ImGuiCond_Always);
	ImGui::Begin("Loading Disrupt Editor", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	const char* messages[]{
		"Reticulating Splines...",
		"I brought this on Clara. Brought her into my mess...",
		"And then go forward and back, then put one foot forward",
		"You wouldn't download a car",
		"Kweh!"
	};
	static int num = time(NULL) % (sizeof(messages) / sizeof(messages[0]));
	ImGui::Text(messages[num]);

	ImGui::ProgressBar(world.loadingProgress, ImVec2(-1.0f, 0.0f), world.loadingStatus.c_str());
	ImGui::End();
}

int main(int argc, char **argv) {
	SetUnhandledExceptionFilter(HandleException);
	SDL_Init(SDL_INIT_EVERYTHING);

	reloadSettings();

	SDL_Window *window = RenderInterface::instance().window;
	if (settings.maximized)
		SDL_MaximizeWindow(window);
	Camera &camera = RenderInterface::instance().camera;
	camera.type = Camera::FLYCAM;
#if 0
	FH::Init();
	CLODataDictionaries::instance();
#endif
#if 0
	{
		FH::Init();
		SDL_RWops* fp = FH::openFileHash(0x49637b42);
		/*SDL_RWops* temp = SDL_RWFromFile("test.cbatch", "wb");
		std::vector<uint8_t> data(SDL_RWsize(fp));
		SDL_RWread(fp, data.data(), 1, data.size());
		SDL_RWwrite(temp, data.data(), 1, data.size());
		SDL_RWclose(temp);*/

		buildingBatchFile b;
		CBinaryArchiveReader reader(fp);
		b.open(reader);
		SDL_RWclose(fp);
		return 1;
	}
#endif

	//DEBUG Save SBAO
	/*{
		SDL_RWops* fp = SDL_RWFromFile("000b1d29.sbao", "wb");
		size_t size = 0;

		Vector< Vector<uint8_t> > layers(3);
		Vector< std::string > layerFiles = {
			"bgm_ex1_alex08 l1.scd.ogg",
			"bgm_ex1_alex08 l2.scd.ogg",
			"bgm_ex1_alex08 l3.scd.ogg",
		};
		for (int i = 0; i < 3; ++i) {
#if 0
			unsigned int channels, sampleRate;
			drwav_uint64 totalSampleCount;
			short* pSampleData = drwav_open_and_read_file_s16(layerFiles[i].c_str(), &channels, &sampleRate, &totalSampleCount);
			layers[i].resize(totalSampleCount * sizeof(short));
			memcpy(layers[i].data(), pSampleData, layers[i].size());
			drwav_free(pSampleData);
#endif
			SDL_RWops* a = SDL_RWFromFile(layerFiles[i].c_str(), "rb");
			layers[i].resize(SDL_RWsize(a));
			SDL_RWread(a, layers[i].data(), 1, SDL_RWsize(a));
			SDL_RWclose(a);
		}
		layers[0].resize(21546520);
		layers[1].resize(10773318);
		layers[2].resize(10773318);

		Vector< Vector<uint8_t> > headers(layers.size());
		Vector< uint8_t* > ptrs(layers.size());

		//Get first 4 packets of ogg as the header
		for (int i = 0; i < layers.size(); ++i) {
			ptrs[i] = layers[i].data() + 4096;
			headers[i].insert(headers[i].end(), layers[i].data(), layers[i].data() + 4096);
		}

		uint32_t maxOgglength = 0;
		for (auto &layer : layers)
			maxOgglength = std::max(maxOgglength, (uint32_t)layer.size());

		uint32_t totalBlocks = (maxOgglength / 162) + 1;

		Vector<uint32_t> infoTable;
		infoTable.push_back(0);//Temporary
		infoTable.push_back(644);//Todo
		for (int i = 0; i < layers.size(); ++i)
			infoTable.push_back(layers[i].end()._Ptr - ptrs[i]);

		struct sbaoHeader {
			uint32_t magic = 207362;
			uint32_t unk1 = 0;
			uint32_t unk2 = 0;
			uint32_t unk3 = 0;
			uint32_t unk4 = 0;
			uint32_t unk5 = 1342177280;
			uint32_t unk6 = 2;
		};
		sbaoHeader head;
		SDL_RWwrite(fp, &head, sizeof(head), 1);
		SDL_WriteLE32(fp, 1048585);//type = interweaved 9 stream
		SDL_WriteLE32(fp, 0);
		SDL_WriteLE32(fp, layers.size());
		SDL_WriteLE32(fp, totalBlocks);//totalBlocks
		SDL_WriteLE32(fp, infoTable.size() * sizeof(uint32_t));//totalInfoSize
		size_t infoOffset = SDL_RWtell(fp);
		SDL_RWwrite(fp, infoTable.data(), sizeof(uint32_t), infoTable.size());
		for (size_t i = 0; i < 64 - layers.size() * 4; ++i)
			SDL_WriteU8(fp, 0);

		//Write Header sizes
		for (int i = 0; i < layers.size(); ++i)
			SDL_WriteLE32(fp, headers[i].size());

		//Write Headers
		for (int i = 0; i < layers.size(); ++i)
			SDL_RWwrite(fp, headers[i].data(), 1, headers[i].size());

		infoTable[0] = SDL_RWtell(fp) - 120;

		//Write Blocks
		for (uint32_t blockI = 0; blockI < totalBlocks; ++blockI) {
			SDL_WriteLE32(fp, 3);//BlockId
			SDL_WriteLE32(fp, blockI == totalBlocks - 1 ? 0 : 644);//unk

																	// Read in the block sizes
			for (unsigned long i = 0; i < layers.size(); i++) {
				uint32_t left = layers[i].end()._Ptr - ptrs[i];
				uint32_t out = std::min(left, (uint32_t)326);
				SDL_LogVerbose(SDL_LOG_CATEGORY_AUDIO, "%u", out);

				SDL_WriteLE32(fp, out);
			}

			for (unsigned long i = 0; i < layers.size(); i++) {
				uint32_t left = layers[i].end()._Ptr - ptrs[i];
				uint32_t out = std::min(left, (uint32_t)326);

				SDL_RWwrite(fp, ptrs[i], 1, out);
				ptrs[i] += out;
			}
		}

		size = SDL_RWtell(fp);

		//Rewrite info table
		SDL_RWseek(fp, infoOffset, RW_SEEK_SET);
		SDL_RWwrite(fp, infoTable.data(), sizeof(uint32_t), infoTable.size());

		SDL_RWclose(fp);
		return 0;
	}*/

	//Start World Loader
	std::thread worldLoaderThread(world.loaderThread);

	Uint32 ticks = SDL_GetTicks();
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

		Frustum frustum(RenderInterface::instance().sceneCB.ViewProjection);

		world.mutex.lock();

		if (world.readyToRender) {
			UI::displayTopMenu();
			UI::displayTempWindows();
			UI::displayWindows();

			if (settings.drawTerrain && world.pd3dCommandList)
				RenderInterface::instance().g_pd3dDeviceContext->ExecuteCommandList(world.pd3dCommandList, TRUE);

			for (auto& it : world.wlus) {
				if (it.first.find(!settings.displayNear ? "_near" : "_far") != std::string::npos) continue;

				//if (glm::distance2(it.second->aabb.maxp - it.second->aabb.minp, RenderInterface::instance().camera.location) > 15.f * 15.f && !frustum.IsBoxVisible(it.second->aabb)) continue;
				//if (glm::distance2(it.second->aabb.maxp - it.second->aabb.minp, RenderInterface::instance().camera.location) > 150.f * 150.f) continue;

				//dd::aabb(&it.second->aabb.minp.x, &it.second->aabb.minp.y, red);

				RenderInterface::instance().g_pd3dDeviceContext->ExecuteCommandList(it.second->plist, TRUE);
			}
		}

		renderProgressBar();

		dd::xzSquareGrid(-50, 50, 0, 1, blue);

		world.mutex.unlock();

		if (!ImGui::IsAnyWindowHovered())
			camera.update(delta);

		RenderInterface::instance().endFrame();

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