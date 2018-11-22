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
	D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &sd, &mDXGISwapChain, &mD3D10Dev);

	// get mWindowRenderTargetTexture2D and create mWindowRenderTargetView
	mDXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&mWindowRenderTargetTexture2D);
	mD3D10Dev->CreateRenderTargetView(mWindowRenderTargetTexture2D, NULL, &mWindowRenderTargetView);
	D3D10_TEXTURE2D_DESC windowRenderTargetTexture2DDesc;
	mWindowRenderTargetTexture2D->GetDesc(&windowRenderTargetTexture2DDesc);

	// create Window Depth Stencil Texture2D
	D3D10_TEXTURE2D_DESC windowDepthStencilTexture2DDesc{ 0 };
	windowDepthStencilTexture2DDesc.Width = windowRenderTargetTexture2DDesc.Width;
	windowDepthStencilTexture2DDesc.Height = windowRenderTargetTexture2DDesc.Height;
	windowDepthStencilTexture2DDesc.MipLevels = windowRenderTargetTexture2DDesc.MipLevels;
	windowDepthStencilTexture2DDesc.ArraySize = windowRenderTargetTexture2DDesc.ArraySize;
	windowDepthStencilTexture2DDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	windowDepthStencilTexture2DDesc.SampleDesc.Count = 1;
	windowDepthStencilTexture2DDesc.SampleDesc.Quality = 0;
	windowDepthStencilTexture2DDesc.Usage = D3D10_USAGE_DEFAULT;
	windowDepthStencilTexture2DDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	windowDepthStencilTexture2DDesc.CPUAccessFlags = 0;
	windowDepthStencilTexture2DDesc.MiscFlags = 0;
	mD3D10Dev->CreateTexture2D(&windowDepthStencilTexture2DDesc, nullptr, &mWindowDepthStencilTexture2D);
	mWindowDepthStencilTexture2D->GetDesc(&windowDepthStencilTexture2DDesc);

	// create Window Depth Stencil View
	D3D10_DEPTH_STENCIL_VIEW_DESC windowDepthStencilViewDesc;
	windowDepthStencilViewDesc.Format = windowDepthStencilTexture2DDesc.Format;
	windowDepthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	windowDepthStencilViewDesc.Texture2D.MipSlice = 0;
	mD3D10Dev->CreateDepthStencilView(mWindowDepthStencilTexture2D, &windowDepthStencilViewDesc, &mWindowDepthStencilView);

	//////////////////////////////////////////////////////////////////////////
	// create Texture2D
	//////////////////////////////////////////////////////////////////////////

	// create Texture2D
	D3D10_TEXTURE2D_DESC pTexture2DDesc{ 0 };
	pTexture2DDesc.Width = 256;
	pTexture2DDesc.Height = 256;
	pTexture2DDesc.MipLevels = 0;
	pTexture2DDesc.ArraySize = 1;
	pTexture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pTexture2DDesc.SampleDesc.Count = 1;
	pTexture2DDesc.SampleDesc.Quality = 0;
	pTexture2DDesc.Usage = D3D10_USAGE_DEFAULT;
	pTexture2DDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	pTexture2DDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	ID3D10Texture2D *ppTexture2D = nullptr;
	mD3D10Dev->CreateTexture2D(&pTexture2DDesc, nullptr, &ppTexture2D);
	ppTexture2D->GetDesc(&pTexture2DDesc);

	// create ShaderResourceView for Texture2D
	D3D10_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = pTexture2DDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = pTexture2DDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	ID3D10ShaderResourceView *ppSRView = nullptr;
	mD3D10Dev->CreateShaderResourceView(ppTexture2D, &shaderResourceViewDesc, &ppSRView);
	mD3D10Dev->GenerateMips(ppSRView);
}

// Created SL-160225
void CAppMain::Destroy()
{
	// window render targets
	mWindowDepthStencilView->Release();
	mWindowDepthStencilTexture2D->Release();
	mWindowRenderTargetView->Release();
	mWindowRenderTargetTexture2D->Release();

	// D3D10 handlers
	mD3D10Dev->Release();

	// DXGI handlers
	mDXGISwapChain->Release();
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
	mD3D10Dev->ClearRenderTargetView(mWindowRenderTargetView, clearColor);
	mD3D10Dev->ClearDepthStencilView(mWindowDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

	// set render target view
	mD3D10Dev->OMSetRenderTargets(1, &mWindowRenderTargetView, mWindowDepthStencilView);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	mDXGISwapChain->Present(0, 0);
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};
