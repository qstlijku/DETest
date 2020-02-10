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
#include "WorldRenderer.h"
#include <Windows.h>
#include <Shellapi.h>
#include <DbgHelp.h>
#include <RoadNetwork.h>
#include <SDL_syswm.h>
#include <imgui_impl_sdl.h>
#include <dr_wav.h>
#include <buildingBatchFile.h>
#include <DB.h>
#include <hkxFile.h>
static LONG WINAPI HandleException(struct _EXCEPTION_POINTERS* apExceptionInfo) {
	HANDLE hFile = ::CreateFile(L"crash.mdmp", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile) {
		_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = apExceptionInfo;
		ExInfo.ClientPointers = FALSE;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		::CloseHandle(hFile);

		MessageBox(NULL, L"DisruptEditor " DE_VERSIONSTR " has crashed! A crash.mdmp has been written",
			L"Your session has been Disrupted!",
			MB_ICONERROR | MB_OK
		);
	}
	exit(0);
}

void renderProgressBar() {
	if (world.loadingProgress >= 1.f)
		return;

	ImGuiIO &io = ImGui::GetIO();

	if (world.readyToRender)
		ImGui::SetNextWindowPos(ImVec2(15, 25), ImGuiCond_Always);
	else
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(400, 78), ImGuiCond_Always);
	ImGui::Begin("Loading Disrupt Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	const char* messages[]{
		"Reticulating Splines...",
		"I brought this on Clara. Brought her into my mess...",
		"And then go forward and back, then put one foot forward",
		"You wouldn't download a car",
		"Kweh!",
	};
	static int num = time(nullptr) % (sizeof(messages) / sizeof(messages[0]));
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
	CBinaryArchiveReader reader(FH::openFile("worlds\\windy_city\\generated\\batchmeshentity\\batchmeshentity_c0_i0_xn2047_yn2559_xp2047_yp2559_phys.cbatch"));
	batchCollisionFile b;
	b.open(reader);
	__debugbreak();
	//CLODataDictionaries::instance();
#endif

	//Start World Loader
	std::thread worldLoaderThread(world.loaderThread);

	Uint32 ticks = SDL_GetTicks();
	bool windowOpen = true;
	while (windowOpen) {
		//Event Handling
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type) {
			case SDL_WINDOWEVENT: {
				switch (event.window.event) {
				case SDL_WINDOWEVENT_CLOSE:
					windowOpen = false;
					saveSettings();
					exit(0);
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					settings.windowSize = glm::ivec2(event.window.data1, event.window.data2);
					saveSettings();
					RenderInterface::instance().onResize();
					world.onResize();
					break;
				}
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

		//Drawing
		float delta = (SDL_GetTicks() - ticks) / 1000.f;
		ticks = SDL_GetTicks();
		if (delta > 0.5f)
			delta = 0.5f;

		RenderInterface &renderInterface = RenderInterface::instance();
		renderInterface.sceneCB.View = glm::lookAt(camera.location, camera.lookingAt, camera.up);
		renderInterface.sceneCB.Projection = glm::perspective(settings.fov, (float)settings.windowSize.x / settings.windowSize.y, settings.near_plane, settings.far_plane);
		renderInterface.sceneCB.ViewProjection = renderInterface.sceneCB.Projection * renderInterface.sceneCB.View;
		RenderInterface::instance().newFrame();

		world.mutex.lock();

		if (world.readyToRender) {
			UI::displayTopMenu();
			UI::displayTempWindows();
			UI::displayWindows();

			world.terrainCursor = glm::vec3(0.f);
			if (settings.drawTerrain && world.terrainCommandList) {
				RenderInterface::instance().g_pd3dDeviceContext->ExecuteCommandList(world.terrainCommandList, TRUE);

				//Get Depth At Cursor
				RenderInterface::instance().g_pd3dDeviceContext->CopySubresourceRegion(RenderInterface::instance().dsBufferCPU.Get(), 0, 0, 0, 0, RenderInterface::instance().dsBuffer.Get(), 0, NULL);

				D3D11_MAPPED_SUBRESOURCE msr;
				HRESULT ret = RenderInterface::instance().g_pd3dDeviceContext->Map(RenderInterface::instance().dsBufferCPU.Get(), 0, D3D11_MAP_READ, 0, &msr);
				
				int posX, posY;
				SDL_GetMouseState(&posX, &posY);

				// copy data
				float depth = 1.f;
				SDL_assert_release(msr.RowPitch == settings.windowSize.x * sizeof(float));
				Sint64 offset = (posY * msr.RowPitch) + (posX * sizeof(float));
				if(msr.pData)
					memcpy(&depth, (uint8_t*)msr.pData + offset, sizeof(depth));

				RenderInterface::instance().g_pd3dDeviceContext->Unmap(RenderInterface::instance().dsBufferCPU.Get(), 0);

				//Convert
				if (depth != 1.f) {
					glm::vec2 texCoord(posX / (float)settings.windowSize.x, 1.f - (posY / (float)settings.windowSize.y));
					glm::vec4 worldPos = glm::inverse(renderInterface.sceneCB.ViewProjection) * glm::vec4(texCoord * 2.f - 1.f, depth, 1.f);
					worldPos /= worldPos.w;
					world.terrainCursor = glm::vec3(worldPos.x, worldPos.y, worldPos.z);
				}
			}

			glm::vec2 camPos2D(RenderInterface::instance().camera.location);
			glm::vec3 camPos3D(RenderInterface::instance().camera.location);
			
			for (auto& it : world.wlus) {
				if (!it.second->gBucket)
					continue;

				if (it.second->wluType == wluFile::WLU_WORLD && settings.displayWorld) {
					it.second->gBucket->draw();
				}

				if ((it.second->wluType == wluFile::WLU_NEAR && settings.displayNear) || (it.second->wluType == wluFile::WLU_FAR && settings.displayFar)) {
					glm::vec3 bbMin(it.second->bbMin, 0);
					glm::vec3 bbMax(it.second->bbMax, 512);
					glm::vec3 pos(bbMin + (bbMax - bbMin) / 2.f);

					bool isIn = (camPos3D.x >= bbMin.x && camPos3D.x <= bbMax.x) &&
						(camPos3D.y >= bbMin.y && camPos3D.y <= bbMax.y) &&
						(camPos3D.z >= bbMin.z && camPos3D.z <= bbMax.z);

					if (isIn)
						it.second->gBucket->draw();

					dd::aabb(&bbMin.x, &bbMax.x, isIn ? blue : red);

					char name[50];
					snprintf(name, sizeof(name), "%u", it.second->sectorID);
					dd::projectedText(name, &pos.x, isIn ? blue : red, &renderInterface.sceneCB.ViewProjection[0][0], 0, 0, renderInterface.sceneCB.windowSize.x, renderInterface.sceneCB.windowSize.y, 0.5f);
				}
			}

			WorldRenderer::draw(renderInterface.g_pd3dDeviceContext.Get());
		}

		renderProgressBar();

		dd::xzSquareGrid(-50, 50, 0, 1, blue);

		world.mutex.unlock();

		if (!ImGui::IsAnyWindowHovered())
			camera.update(delta);

		RenderInterface::instance().endFrame();
	}

	ImGui::DestroyContext();
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}