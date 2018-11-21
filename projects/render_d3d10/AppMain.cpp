#include "AppMain.hpp"
#include <cinttypes>
#include <d3d9types.h>

// vertex structure
struct CUSTOMVERTEX { FLOAT X, Y, Z, W; DWORD COLOR; FLOAT U, V; };

// vertex array
CUSTOMVERTEX vertices[] = {
	{ +1.0f, -1.0f, +0.0f, +1.0, D3DCOLOR_XRGB(0, 0, 255),   1, 0 },
	{ +1.0f, +1.0f, +0.0f, +1.0, D3DCOLOR_XRGB(0, 255, 0),   1, 1 },
	{ -1.0f, -1.0f, +0.0f, +1.0, D3DCOLOR_XRGB(255, 0, 0),   0, 0 },
	{ -1.0f, +1.0f, +0.0f, +1.0, D3DCOLOR_XRGB(255, 255, 0), 0, 1 },
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

	// create swap chain description
	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferCount = 1;
	sd.BufferDesc.Width = mViewportWidth;
	sd.BufferDesc.Height = mViewportHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	// create swap chain and device
	D3D10CreateDevice(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &mD3D10Dev);

	// create swap chain factory
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&mDXGIFactory));

	// create swap chain
	mDXGIFactory->CreateSwapChain(mD3D10Dev, &sd, &mDXGISwapChain);

	// create render target view
	mDXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&mSwapChainBuffer);
	mD3D10Dev->CreateRenderTargetView(mSwapChainBuffer, NULL, &mRenderTargetView);
}

// Created SL-160225
void CAppMain::Destroy()
{
	mRenderTargetView->Release();
	mSwapChainBuffer->Release();
	mD3D10Dev->Release();
	mDXGISwapChain->Release();
	mDXGIFactory->Release();
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
	D3DXMatrixPerspectiveFovRH(&matProj, (FLOAT)D3DXToRadian(45.0f), (FLOAT)mViewportWidth / mViewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	D3DXMATRIX WVP;
	WVP = matWorld * matView * matProj;

	// Setup the viewport
	D3D10_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = mViewportHeight;
	vp.Height = mViewportWidth;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	mD3D10Dev->RSSetViewports(1, &vp);

	// Clear the back buffer 
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	mD3D10Dev->ClearRenderTargetView(mRenderTargetView, clearColor);

	// set render target view
	mD3D10Dev->OMSetRenderTargets(1, &mRenderTargetView, NULL);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	mDXGISwapChain->Present(0, 0);
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};
