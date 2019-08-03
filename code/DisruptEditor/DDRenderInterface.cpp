#include "DDRenderInterface.h"

#include "Common.h"
#include <SDL.h>
#include "stb_image_write.h"
#include "Version.h"
#include <SDL_syswm.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_dx11.h>
#include <ImGuizmo.h>
#include <d3dcompiler.h>

RenderInterface::RenderInterface() {
	window = SDL_CreateWindow("Disrupt Editor v" DE_VERSIONSTR, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, settings.windowSize.x, settings.windowSize.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	if (window == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", SDL_GetError(), NULL);
		exit(0);
	}

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);

	if (!CreateDeviceD3D(info.info.win.window)) {
		//CleanupDeviceD3D();
		exit(1);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplSDL2_InitForD3D(window);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
	ImGui::StyleColorsDark(NULL);
	dd::initialize(this);

	//Load Shaders
	lines = loadShader("DebugLines");
	tex = loadShader("DebugTex");
	terrain = loadShader("Terrain");

	//Setup Constant Buffers
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(sceneCB), D3D11_BIND_CONSTANT_BUFFER);
	g_pd3dDevice->CreateBuffer(
		&constantBufferDesc,
		NULL,
		&sceneCBB);
	g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &sceneCBB);

	CD3D11_BUFFER_DESC constantBufferDesc2(sizeof(objectCB), D3D11_BIND_CONSTANT_BUFFER);
	g_pd3dDevice->CreateBuffer(
		&constantBufferDesc2,
		NULL,
		&objectCBB);
	g_pd3dDeviceContext->VSSetConstantBuffers(1, 1, &objectCBB);

	//Texture Sampler
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	g_pd3dDevice->CreateSamplerState(&samplerDesc, &tex0);

	//Raster State
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = true;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthClipEnable = true;
	rsDesc.ScissorEnable = false;
	rsDesc.MultisampleEnable = false;
	rsDesc.AntialiasedLineEnable = false;
	g_pd3dDevice->CreateRasterizerState(&rsDesc, rasterizerState.GetAddressOf());
}

void RenderInterface::newFrame() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	g_pd3dDeviceContext->UpdateSubresource(sceneCBB, 0, NULL, &sceneCB, 0, 0);

	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(D3D11_VIEWPORT));
	vp.Width = sceneCB.windowSize.x;
	vp.Height = sceneCB.windowSize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = vp.TopLeftY = 0;
	g_pd3dDeviceContext->RSSetViewports(1, &vp);
	g_pd3dDeviceContext->RSSetState(rasterizerState.Get());
}

void RenderInterface::endFrame() {
	dd::flush(0);
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	g_pSwapChain->Present(1, 0);
}

void RenderInterface::CreateRenderTarget() {
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

bool RenderInterface::CreateDeviceD3D(HWND hWnd) {
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void RenderInterface::beginDraw() {
	//Raster State
	static Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	if (!rasterizerState) {
		D3D11_RASTERIZER_DESC rsDesc = {};
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = true;
		rsDesc.DepthBias = 0;
		rsDesc.DepthBiasClamp = 0.0f;
		rsDesc.SlopeScaledDepthBias = 0.0f;
		rsDesc.DepthClipEnable = true;
		rsDesc.ScissorEnable = false;
		rsDesc.MultisampleEnable = false;
		rsDesc.AntialiasedLineEnable = false;
		g_pd3dDevice->CreateRasterizerState(&rsDesc, rasterizerState.GetAddressOf());
	}
	g_pd3dDeviceContext->RSSetState(rasterizerState.Get());
}

void RenderInterface::endDraw() {
}

void RenderInterface::drawPointList(const dd::DrawVertex *points, int count, bool depthEnabled) {
	//Set Shaders
	g_pd3dDeviceContext->VSSetShader(lines.pVertexShader, NULL, NULL);
	g_pd3dDeviceContext->PSSetShader(lines.pPixelShader, NULL, NULL);

	//Setup Vertex Description
	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	if (!m_inputLayout)
		g_pd3dDevice->CreateInputLayout(
			vertexDesc,
			ARRAYSIZE(vertexDesc),
			lines.vShaderBlob->GetBufferPointer(),
			lines.vShaderBlob->GetBufferSize(),
			&m_inputLayout);

	//Setup Vertex Buffer
	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = points;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(dd::DrawVertex) * count, D3D11_BIND_VERTEX_BUFFER);

	ID3D11Buffer* m_vertexBuffer = NULL;
	g_pd3dDevice->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&m_vertexBuffer);

	UINT stride = sizeof(dd::DrawVertex);
	UINT offset = 0;
	g_pd3dDeviceContext->IASetVertexBuffers(
		0,
		1,
		&m_vertexBuffer,
		&stride,
		&offset);

	//Draw
	g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	g_pd3dDeviceContext->IASetInputLayout(m_inputLayout.Get());
	g_pd3dDeviceContext->Draw(count, 0);

	m_vertexBuffer->Release();
}

void RenderInterface::drawLineList(const dd::DrawVertex *points, int count, bool depthEnabled) {
	//Set Shaders
	g_pd3dDeviceContext->VSSetShader(lines.pVertexShader, NULL, NULL);
	g_pd3dDeviceContext->PSSetShader(lines.pPixelShader, NULL, NULL);

	//Setup Vertex Description
	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	if (!m_inputLayout)
		g_pd3dDevice->CreateInputLayout(
			vertexDesc,
			ARRAYSIZE(vertexDesc),
			lines.vShaderBlob->GetBufferPointer(),
			lines.vShaderBlob->GetBufferSize(),
			&m_inputLayout);

	//Setup Vertex Buffer
	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = points;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(dd::DrawVertex) * count, D3D11_BIND_VERTEX_BUFFER);

	ID3D11Buffer* m_vertexBuffer = NULL;
	g_pd3dDevice->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&m_vertexBuffer);

	UINT stride = sizeof(dd::DrawVertex);
	UINT offset = 0;
	g_pd3dDeviceContext->IASetVertexBuffers(
		0,
		1,
		&m_vertexBuffer,
		&stride,
		&offset);

	//Draw
	g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	g_pd3dDeviceContext->IASetInputLayout(m_inputLayout.Get());
	g_pd3dDeviceContext->Draw(count, 0);

	m_vertexBuffer->Release();
}

void RenderInterface::drawGlyphList(const dd::DrawVertex * glyphs, int count, dd::GlyphTextureHandle glyphTex) {
	//Set Shaders
	g_pd3dDeviceContext->VSSetShader(tex.pVertexShader, NULL, NULL);
	g_pd3dDeviceContext->PSSetShader(tex.pPixelShader, NULL, NULL);

	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
	  { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	if (!m_inputLayout)
		g_pd3dDevice->CreateInputLayout(
			vertexDesc,
			ARRAYSIZE(vertexDesc),
			tex.vShaderBlob->GetBufferPointer(),
			tex.vShaderBlob->GetBufferSize(),
			&m_inputLayout);

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = glyphs;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(dd::DrawVertex) * count, D3D11_BIND_VERTEX_BUFFER);

	ID3D11Buffer* m_vertexBuffer = NULL;
	g_pd3dDevice->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&m_vertexBuffer);

	UINT stride = sizeof(dd::DrawVertex);
	UINT offset = 0;
	g_pd3dDeviceContext->IASetVertexBuffers(
		0,
		1,
		&m_vertexBuffer,
		&stride,
		&offset);

	Texture* pTexture = (Texture*)glyphTex;
	g_pd3dDeviceContext->PSSetShaderResources(0, 1, &pTexture->pResource);
	g_pd3dDeviceContext->PSSetSamplers(0, 1, &tex0);

	static Microsoft::WRL::ComPtr<ID3D11BlendState> blendStateText;
	if (!blendStateText) {
		D3D11_BLEND_DESC bsDesc = {};
		bsDesc.RenderTarget[0].BlendEnable = true;
		bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		g_pd3dDevice->CreateBlendState(&bsDesc, blendStateText.GetAddressOf());
	}
	const float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	g_pd3dDeviceContext->OMSetBlendState(blendStateText.Get(), blendFactor, 0xFFFFFFFF);

	g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pd3dDeviceContext->IASetInputLayout(m_inputLayout.Get());
	g_pd3dDeviceContext->Draw(count, 0);

	g_pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	m_vertexBuffer->Release();
}

RenderInterface::Shader RenderInterface::loadShader(const std::string &name) {
	RenderInterface::Shader shader;

	//Create Vertex Shader
	{
		std::string filename = "res/shaders/" + name + "V.hlsl";
		std::string contents = readFile(filename);
		D3DCompile(contents.c_str(), contents.size(), filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", 0, 0, &shader.vShaderBlob, NULL);
		if (shader.vShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
			return shader;
		if (g_pd3dDevice->CreateVertexShader((DWORD*)shader.vShaderBlob->GetBufferPointer(), shader.vShaderBlob->GetBufferSize(), NULL, &shader.pVertexShader) != S_OK)
			return shader;
	}

	//Create Pixel Shader
	{
		std::string filename = "res/shaders/" + name + "P.hlsl";
		std::string contents = readFile(filename);
		D3DCompile(contents.c_str(), contents.size(), filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", 0, 0, &shader.pShaderBlob, NULL);
		if (shader.pShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
			return shader;
		if (g_pd3dDevice->CreatePixelShader((DWORD*)shader.pShaderBlob->GetBufferPointer(), shader.pShaderBlob->GetBufferSize(), NULL, &shader.pPixelShader) != S_OK)
			return shader;
	}

	return shader;
}

dd::GlyphTextureHandle RenderInterface::createGlyphTexture(int width, int height, const void * pixels) {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA textureBufferData = { 0 };
	textureBufferData.pSysMem = pixels;
	textureBufferData.SysMemPitch = width;

	Texture* pTexture = new Texture;
	g_pd3dDevice->CreateTexture2D(&desc, &textureBufferData, &pTexture->pTexture);
	g_pd3dDevice->CreateShaderResourceView(pTexture->pTexture, NULL, &pTexture->pResource);
	return (dd::GlyphTextureHandle)pTexture;
}

void RenderInterface::destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) {
	Texture* pTexture = (Texture*)glyphTex;
	pTexture->pTexture->Release();
	pTexture->pResource->Release();
	delete pTexture;
}

RenderInterface& RenderInterface::instance() {
	static RenderInterface ri;
	return ri;
}
