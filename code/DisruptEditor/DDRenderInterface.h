#pragma once

#include "Colors.h"
#include "glm/glm.hpp"
#include <SDL_video.h>
#include <SDL_mutex.h>
#include "Camera.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>

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

	void CreateDeviceD3D(HWND hWnd);
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void onResize();

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
	Shader lines, tex, model, terrain, spline;
	Shader loadShader(const std::string& name);

	Camera camera;

	SDL_Window* window;

	Microsoft::WRL::ComPtr<ID3D11Device> g_pd3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pd3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> g_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_mainRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> dsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> dsBufferCPU;

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

	struct DDStr {
		std::string str;
		glm::vec3 pos;
	};
	std::vector<DDStr> ddstrs;
	void pushDebugStr(const char* str, const glm::vec3& pos);

	static RenderInterface& instance();
};

//From https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644

struct AABB {
	glm::vec3 minp = glm::vec3(0);
	glm::vec3 maxp = glm::vec3(0);
};

class Frustum {
public:
	Frustum() {}

	// m = ProjectionMatrix * ViewMatrix 
	Frustum(glm::mat4 m);

	// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	bool IsBoxVisible(const glm::vec3& minp, const glm::vec3& maxp) const;
	bool IsBoxVisible(const AABB& aabb) const {
		return IsBoxVisible(aabb.minp, aabb.maxp);
	}

private:
	enum Planes {
		Left = 0,
		Right,
		Bottom,
		Top,
		Near,
		Far,
		Count,
		Combinations = Count * (Count - 1) / 2
	};

	template<Planes i, Planes j>
	struct ij2k {
		enum { k = i * (9 - i) / 2 + j - 1 };
	};

	template<Planes a, Planes b, Planes c>
	glm::vec3 intersection(const glm::vec3* crosses) const;

	glm::vec4   m_planes[Count];
	glm::vec3   m_points[8];
};

inline Frustum::Frustum(glm::mat4 m) {
	m = glm::transpose(m);
	m_planes[Left] = m[3] + m[0];
	m_planes[Right] = m[3] - m[0];
	m_planes[Bottom] = m[3] + m[1];
	m_planes[Top] = m[3] - m[1];
	m_planes[Near] = m[3] + m[2];
	m_planes[Far] = m[3] - m[2];

	glm::vec3 crosses[Combinations] = {
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Right])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Bottom])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Top])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Left]),   glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Bottom])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Top])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Right]),  glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Bottom]), glm::vec3(m_planes[Top])),
		glm::cross(glm::vec3(m_planes[Bottom]), glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Bottom]), glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Top]),    glm::vec3(m_planes[Near])),
		glm::cross(glm::vec3(m_planes[Top]),    glm::vec3(m_planes[Far])),
		glm::cross(glm::vec3(m_planes[Near]),   glm::vec3(m_planes[Far]))
	};

	m_points[0] = intersection<Left, Bottom, Near>(crosses);
	m_points[1] = intersection<Left, Top, Near>(crosses);
	m_points[2] = intersection<Right, Bottom, Near>(crosses);
	m_points[3] = intersection<Right, Top, Near>(crosses);
	m_points[4] = intersection<Left, Bottom, Far>(crosses);
	m_points[5] = intersection<Left, Top, Far>(crosses);
	m_points[6] = intersection<Right, Bottom, Far>(crosses);
	m_points[7] = intersection<Right, Top, Far>(crosses);

}

// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
inline bool Frustum::IsBoxVisible(const glm::vec3& minp, const glm::vec3& maxp) const {
	// check box outside/inside of frustum
	for (int i = 0; i < Count; i++) {
		if ((glm::dot(m_planes[i], glm::vec4(minp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(maxp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(minp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(maxp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(minp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(maxp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(minp.x, maxp.y, maxp.z, 1.0f)) < 0.0) &&
			(glm::dot(m_planes[i], glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f)) < 0.0)) {
			return false;
		}
	}

	// check frustum outside/inside box
	int out;
	out = 0; for (int i = 0; i < 8; i++) out += ((m_points[i].x > maxp.x) ? 1 : 0); if (out == 8) return false;
	out = 0; for (int i = 0; i < 8; i++) out += ((m_points[i].x < minp.x) ? 1 : 0); if (out == 8) return false;
	out = 0; for (int i = 0; i < 8; i++) out += ((m_points[i].y > maxp.y) ? 1 : 0); if (out == 8) return false;
	out = 0; for (int i = 0; i < 8; i++) out += ((m_points[i].y < minp.y) ? 1 : 0); if (out == 8) return false;
	out = 0; for (int i = 0; i < 8; i++) out += ((m_points[i].z > maxp.z) ? 1 : 0); if (out == 8) return false;
	out = 0; for (int i = 0; i < 8; i++) out += ((m_points[i].z < minp.z) ? 1 : 0); if (out == 8) return false;

	return true;
}

template<Frustum::Planes a, Frustum::Planes b, Frustum::Planes c>
inline glm::vec3 Frustum::intersection(const glm::vec3* crosses) const {
	float D = glm::dot(glm::vec3(m_planes[a]), crosses[ij2k<b, c>::k]);
	glm::vec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
		glm::vec3(m_planes[a].w, m_planes[b].w, m_planes[c].w);
	return res * (-1.0f / D);
}