////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"
#include <DDRenderInterface.h>


ApplicationClass::ApplicationClass()
{
	m_Model = 0;
	m_TextureShader = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}

ID3D11Device* GetDevice()
{
	return RenderInterface::instance().g_pd3dDevice.Get();
}

ID3D11DeviceContext* GetDeviceContext()
{
	return RenderInterface::instance().g_pd3dDeviceContext.Get();
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight)
{
	char textureFilename[128];
	bool result;

	// Create and initialize the model object.
	m_Model = new ModelClass;

	// Set the name of the texture file that we will be loading.
	strcpy_s(textureFilename, "../../code/EngineTest/data/stone01.tga");

	result = m_Model->Initialize(GetDevice(), GetDeviceContext(), textureFilename);
	if(!result)
	{
		return false;
	}

	// Create and initialize the texture shader object.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(GetDevice());
	if(!result)
	{
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the texture shader object.
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the model object.
	if(m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	return;
}


bool ApplicationClass::Frame()
{
	bool result;


	// Render the graphics scene.
	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}

XMVECTOR GetXVFromGLM(glm::vec4 v)
{
	XMFLOAT4 temp;
	temp.x = v.x;
	temp.y = v.y;
	temp.z = v.z;
	temp.w = v.w;

	// Load it into a XMVECTOR structure.
	XMVECTOR xv = XMLoadFloat4(&temp);
	return xv;
}

XMMATRIX GetXMFromGLM(glm::mat4 mat)
{
	XMMATRIX xm;
	xm.r[0] = GetXVFromGLM(mat[0]);
	xm.r[1] = GetXVFromGLM(mat[1]);
	xm.r[2] = GetXVFromGLM(mat[2]);
	xm.r[3] = GetXVFromGLM(mat[3]);
	return xm;
}

bool ApplicationClass::Render()
{
	// Clear the buffers to begin the scene.
	//m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	//m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	//m_Direct3D->GetWorldMatrix(worldMatrix);
	//XMMATRIX worldMatrix = XMMatrixIdentity();
	//m_Camera->GetViewMatrix(viewMatrix);
	//projectionMatrix = GetXMFromGLM(RenderInterface::instance().sceneCB.Projection);
	//m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Model->Render(GetDeviceContext());

	// Render the model using the texture shader.
	bool result = m_TextureShader->Render(GetDeviceContext(), m_Model->GetIndexCount(), m_Model->GetTexture());
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	//m_Direct3D->EndScene();

	return true;
}