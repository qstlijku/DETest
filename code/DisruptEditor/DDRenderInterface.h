#pragma once

#include "Colors.h"
#include "glm/glm.hpp"
#include <SDL_video.h>
#include <SDL_mutex.h>
#include "Camera.h"
#include <d3d11.h>
#include <wrl.h>

class VertexBuffer {
public:
	~VertexBuffer() {
		if(pVertexBuffer)
			pVertexBuffer->Release();
	}
	ID3D11Buffer* pVertexBuffer = NULL;
	UINT stride = 0;
};

class IndexBuffer {
public:
	~IndexBuffer() {
		if(pIndexBuffer)
			pIndexBuffer->Release();
	}
	ID3D11Buffer* pIndexBuffer = NULL;
	UINT size = 0;
};

class Texture {
public:
	~Texture() {
		if(pTexture)
			pTexture->Release();
		if(pResource)
			pResource->Release();
	}
	ID3D11Resource* pTexture = NULL;
	ID3D11ShaderResourceView* pResource = NULL;
};

class RenderInterface : public dd::RenderInterface {
public:
	RenderInterface();

	void newFrame();
	void endFrame();
	void setupState(ID3D11DeviceContext* context);

	//
	// These are called by dd::flush() before any drawing and after drawing is finished.
	// User can override these to perform any common setup for subsequent draws and to
	// cleanup afterwards. By default, no-ops stubs are provided.
	//
	void beginDraw();
	void endDraw();

	//
	// Create/free the glyph bitmap texture used by the debug text drawing functions.
	// The debug renderer currently only creates one of those on startup.
	//
	// You're not required to implement these two if you don't care about debug text drawing.
	// Default no-op stubs are provided by default, which disable debug text rendering.
	//
	// Texture dimensions are in pixels, data format is always 8-bits per pixel (Grayscale/GL_RED).
	// The pixel values range from 255 for a pixel within a glyph to 0 for a transparent pixel.
	// If createGlyphTexture() returns null, the renderer will disable all text drawing functions.
	//
	dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void * pixels);
	void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex);

	bool CreateDeviceD3D(HWND hWnd);
	void CreateRenderTarget();

	//
	// Batch drawing methods for the primitives used by the debug renderer.
	// If you don't wish to support a given primitive type, don't override the method.
	//
	void drawPointList(const dd::DrawVertex * points, int count, bool depthEnabled);
	void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled);
	void drawGlyphList(const dd::DrawVertex * glyphs, int count, dd::GlyphTextureHandle glyphTex);

	struct Shader {
		ID3D10Blob* vShaderBlob = NULL;
		ID3D11VertexShader* pVertexShader = NULL;

		ID3D10Blob* pShaderBlob = NULL;
		ID3D11PixelShader* pPixelShader = NULL;
	};
	Shader lines, tex, model, terrain;
	Shader loadShader(const std::string& name);

	Camera camera;

	SDL_Window* window;

	ID3D11Device* g_pd3dDevice = NULL;
	ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;
	ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_depthStencilView;

	struct SceneConstantBuffer {
		glm::mat4 View;
		glm::mat4 Projection;
		glm::mat4 ViewProjection;
		glm::vec2 windowSize;
		glm::vec2 pad;
	} sceneCB;
	ID3D11Buffer* sceneCBB;

	struct ObjectConstantBuffer {
		glm::mat4 Model;
		glm::vec4 Offset;
	} objectCB;
	ID3D11Buffer* objectCBB;
	void updateObjectCBB();

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	ID3D11SamplerState* tex0;

	void onWindowResized();

	static RenderInterface& instance();
};
