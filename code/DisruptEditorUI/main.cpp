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
	if(!IsDebuggerPresent())
		SDL_LogSetOutputFunction(LogOutputFunction, fopen("DisruptEditor.log", "wb"));

	reloadSettings();

	SDL_Window *window = RenderInterface::instance().window;
	if (settings.maximized)
		SDL_MaximizeWindow(window);
	Camera &camera = RenderInterface::instance().camera;
	camera.type = Camera::FLYCAM;

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

		world.mutex.lock();

		if (world.readyToRender) {
			UI::displayTopMenu();
			UI::displayTempWindows();
			UI::displayWindows();

			if (settings.drawTerrain && world.pd3dCommandList)
				RenderInterface::instance().g_pd3dDeviceContext->ExecuteCommandList(world.pd3dCommandList, TRUE);

			for(auto &it : world.wluLists)
				RenderInterface::instance().g_pd3dDeviceContext->ExecuteCommandList(it, TRUE);
		} else {
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

			dd::xzSquareGrid(-50, 50, 0, 1, blue);
		}

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

		//ImGui::InputFloat3("CameraPos", &RenderInterface::instance().camera.location.x);

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