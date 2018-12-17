#include "AppMain.hpp"
#include <cinttypes>

// vertex structure
struct CUSTOMVERTEX { FLOAT X, Y, Z, W; FLOAT R, G, B, A; FLOAT U, V; };

// vertex array
CUSTOMVERTEX vertices[] = {
	{ +1.0f, -1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+1, +0 },
	{ +1.0f, +1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+1, +1 },
	{ -1.0f, -1.0f, +0.0f, +1.0, /**/+1.0f, +0.0f, +0.0f, +1.0, /**/+0, +0 },
	{ -1.0f, +1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+0, +1 },
};

// index array
uint16_t indexes[] = { 0, 1, 2, 2, 1, 3 };

// Created SL-160225
void CAppMain::Init(const HWND hWnd)
{
	// get client rect
	RECT clientWindowRect{ 0 };
	GetClientRect(hWnd, &clientWindowRect);
	mViewportWidth = (WORD)clientWindowRect.right - (WORD)clientWindowRect.left;
	mViewportHeight = (WORD)clientWindowRect.bottom - (WORD)clientWindowRect.top;

	// create Direct3D
	mD3D9 = Direct3DCreate9(D3D_SDK_VERSION);

	// create present parameters
	D3DPRESENT_PARAMETERS d3dpp{ 0 };
	d3dpp.BackBufferWidth = mViewportWidth;
	d3dpp.BackBufferHeight = mViewportHeight;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// create device
	mD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &mD3D9Dev);

	// get device caps
	D3DCAPS9 caps9;
	mD3D9Dev->GetDeviceCaps(&caps9);

	//////////////////////////////////////////////////////////////////////////////
	// Create Texture
	//////////////////////////////////////////////////////////////////////////////

	D3DXCreateTextureFromFile(mD3D9Dev, L"textures/texture.png", &mTexture0);
	mTexture0->GenerateMipSubLevels();

	//////////////////////////////////////////////////////////////////////////////
	// Create Vertex Buffer
	//////////////////////////////////////////////////////////////////////////////

	// create vertex buffer
	mD3D9Dev->CreateVertexBuffer(sizeof(vertices), 0, 0, D3DPOOL_MANAGED, &mVertexBuffer, NULL);

	// fill vertex buffer
	void* pVertexData = nullptr;
	mVertexBuffer->Lock(0, 0, &pVertexData, 0);
	memcpy(pVertexData, vertices, sizeof(vertices));
	mVertexBuffer->Unlock();

	//////////////////////////////////////////////////////////////////////////////
	// Create Index Buffer
	//////////////////////////////////////////////////////////////////////////////

	// create index buffer
	mD3D9Dev->CreateIndexBuffer(36 * sizeof(uint16_t), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIndexBuffer, NULL);

	// fill index buffer
	void* pIndexData = nullptr;
	mIndexBuffer->Lock(0, 0, &pIndexData, 0);
	memcpy(pIndexData, indexes, sizeof(indexes));
	mIndexBuffer->Unlock();

	//////////////////////////////////////////////////////////////////////////////
	// Create Vertex Declaration
	//////////////////////////////////////////////////////////////////////////////

	// declaration
	D3DVERTEXELEMENT9 declaration[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	mD3D9Dev->CreateVertexDeclaration(declaration, &mVertexDeclaration);

	//////////////////////////////////////////////////////////////////////////////
	// Create Vertex Shader
	//////////////////////////////////////////////////////////////////////////////

	ID3DXBuffer* vsBuffer = nullptr;
	ID3DXBuffer* vsErrorBuffer = nullptr;
	ID3DXConstantTable* vsConstantTable = nullptr;
	D3DXCompileShaderFromFile(L"shaders/vs.hlsl", NULL, NULL, "main", "vs_3_0", 0, &vsBuffer, &vsErrorBuffer, &vsConstantTable);

	mD3D9Dev->CreateVertexShader((DWORD *)vsBuffer->GetBufferPointer(), &mVertexShader);

	if (vsConstantTable) vsConstantTable->Release();
	if (vsErrorBuffer) vsErrorBuffer->Release();
	if (vsBuffer) vsBuffer->Release();

	//////////////////////////////////////////////////////////////////////////////
	// Create Pixel Shader
	//////////////////////////////////////////////////////////////////////////////

	ID3DXBuffer* psBuffer = nullptr;
	ID3DXBuffer* psErrorBuffer = nullptr;
	ID3DXConstantTable* psConstantTable = nullptr;
	D3DXCompileShaderFromFile(L"shaders/ps.hlsl", NULL, NULL, "main", "ps_3_0", 0, &psBuffer, &psErrorBuffer, &psConstantTable);

	mD3D9Dev->CreatePixelShader((DWORD *)psBuffer->GetBufferPointer(), &mPixelShader);

	if (psConstantTable) psConstantTable->Release();
	if (psErrorBuffer) psErrorBuffer->Release();
	if (psBuffer) psBuffer->Release();
}

// Created SL-160225
void CAppMain::Destroy()
{
	mPixelShader->Release();
	mVertexShader->Release();
	mVertexDeclaration->Release();
	mIndexBuffer->Release();
	mVertexBuffer->Release();
	mTexture0->Release();
	mD3D9Dev->Release();
	mD3D9->Release();
}

// Created SL-160225
void CAppMain::Render()
{
	// mat world
	D3DXMATRIX matRotate;
	D3DXMATRIX matScale;
	D3DXMATRIX matTranslate;
	D3DXMatrixRotationZ(&matRotate, 0.0f);
	D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
	D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, 0.0f);
	D3DXMATRIX matWorld = matRotate * matScale * matTranslate;

	// mat view
	D3DXMATRIX matView;
	D3DXMatrixLookAtRH(&matView,
		&D3DXVECTOR3(0.0f, 0.0f, 10.0f), // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),  // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)   // the up direction
	);

	// mat projection
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovRH(&matProj, (FLOAT)D3DXToRadian(45), (FLOAT)mViewportWidth / mViewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	D3DXMATRIX WVP;
	WVP = matWorld * matView * matProj;

	// clear
	mD3D9Dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(1, 36, 86), 1.0f, 0);

	// set viewport
	D3DVIEWPORT9 viewport{ 0 };
	viewport.MaxZ = 10.0f;
	viewport.MinZ = 1.0f;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Height = mViewportHeight;
	viewport.Width = mViewportWidth;
	mD3D9Dev->SetViewport(&viewport);

	// draw scene
	mD3D9Dev->BeginScene();

	// device settings
	mD3D9Dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//mD3D9Dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	mD3D9Dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	// uniform float4x4 uWVP : register(c0);
	mD3D9Dev->SetVertexShader(mVertexShader);
	mD3D9Dev->SetVertexShaderConstantF(0, WVP, 4);

	// uniform float4 uColor0 : register(c0);
	// uniform float4 uColor1 : register(c1);
	float color0[] = { 1.0f, 0.5f, 0.2f, 1.0f };
	float color1[] = { 0.0f, 0.5f, 0.2f, 1.0f };
	mD3D9Dev->SetPixelShader(mPixelShader);
	mD3D9Dev->SetPixelShaderConstantF(0, color0, 1);
	mD3D9Dev->SetPixelShaderConstantF(1, color1, 1);

	// setup textures
	mD3D9Dev->SetTexture(0, mTexture0);
	mD3D9Dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	mD3D9Dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	mD3D9Dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	mD3D9Dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	mD3D9Dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// setup vertex stream, index buffer and vertex declarations
	mD3D9Dev->SetVertexDeclaration(mVertexDeclaration);
	mD3D9Dev->SetIndices(mIndexBuffer);
	mD3D9Dev->SetStreamSource(0, mVertexBuffer, 0, sizeof(CUSTOMVERTEX));
	mD3D9Dev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	//mD3D9Dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2);

	//end scene
	mD3D9Dev->EndScene();

	// flush
	mD3D9Dev->Present(NULL, NULL, NULL, NULL);
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};
