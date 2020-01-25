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
	CLODataDictionaries::instance();
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

		Frustum frustum(RenderInterface::instance().sceneCB.ViewProjection);

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

			for (auto& it : world.wlus) {
				if (it.first.find(!settings.displayNear ? "_near" : "_far") != std::string::npos) continue;

				//if (glm::distance2(it.second->aabb.maxp - it.second->aabb.minp, RenderInterface::instance().camera.location) > 15.f * 15.f && !frustum.IsBoxVisible(it.second->aabb)) continue;
				//if (glm::distance2(it.second->aabb.maxp - it.second->aabb.minp, RenderInterface::instance().camera.location) > 150.f * 150.f) continue;

				//dd::aabb(&it.second->aabb.minp.x, &it.second->aabb.minp.y, red);

				if(it.second->plist)
					RenderInterface::instance().g_pd3dDeviceContext->ExecuteCommandList(it.second->plist, TRUE);
			}
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