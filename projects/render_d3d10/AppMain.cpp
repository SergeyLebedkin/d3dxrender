#include "AppMain.hpp"
#include <iostream>
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
	D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &sd, &mDXGISwapChain, &mD3D10Dev);

	// get mWindowRenderTargetTexture2D and create mWindowRenderTargetView
	mDXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&mWindowRenderTargetTexture2D);
	mD3D10Dev->CreateRenderTargetView(mWindowRenderTargetTexture2D, NULL, &mWindowRenderTargetView);
	D3D10_TEXTURE2D_DESC descRTT;
	mWindowRenderTargetTexture2D->GetDesc(&descRTT);

	//////////////////////////////////////////////////////////////////////////
	// create Window Depth Stencil Texture2D and Window Depth Stencil View
	//////////////////////////////////////////////////////////////////////////

	// create Window Depth Stencil Texture2D
	D3D10_TEXTURE2D_DESC descDST{ 0 };
	descDST.Width = descRTT.Width;
	descDST.Height = descRTT.Height;
	descDST.MipLevels = descRTT.MipLevels;
	descDST.ArraySize = descRTT.ArraySize;
	descDST.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDST.SampleDesc.Count = 1;
	descDST.SampleDesc.Quality = 0;
	descDST.Usage = D3D10_USAGE_DEFAULT;
	descDST.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	descDST.CPUAccessFlags = 0;
	descDST.MiscFlags = 0;
	mD3D10Dev->CreateTexture2D(&descDST, nullptr, &mWindowDepthStencilTexture2D);

	// create Window Depth Stencil View
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDST.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	mD3D10Dev->CreateDepthStencilView(mWindowDepthStencilTexture2D, &descDSV, &mWindowDepthStencilView);

	//////////////////////////////////////////////////////////////////////////
	// create Texture2D and Texture2DShaderResourceView
	//////////////////////////////////////////////////////////////////////////

	// create Texture2D
	D3D10_TEXTURE2D_DESC descTex2D{ 0 };
	descTex2D.Width = 256;
	descTex2D.Height = 256;
	descTex2D.MipLevels = 0;
	descTex2D.ArraySize = 1;
	descTex2D.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descTex2D.SampleDesc.Count = 1;
	descTex2D.SampleDesc.Quality = 0;
	descTex2D.Usage = D3D10_USAGE_DEFAULT;
	descTex2D.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	descTex2D.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	descTex2D.MiscFlags = 0;
	mD3D10Dev->CreateTexture2D(&descTex2D, nullptr, &mTexture2D);
	mTexture2D->GetDesc(&descTex2D);

	// create ShaderResourceView for Texture2D
	D3D10_SHADER_RESOURCE_VIEW_DESC descSRV;
	descSRV.Format = descTex2D.Format;
	descSRV.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = descTex2D.MipLevels;
	descSRV.Texture2D.MostDetailedMip = 0;
	mD3D10Dev->CreateShaderResourceView(mTexture2D, &descSRV, &mTexture2DShaderResourceView);

	//////////////////////////////////////////////////////////////////////////
	// create Vertex Buffer
	//////////////////////////////////////////////////////////////////////////

	D3D10_BUFFER_DESC descVertexBuffer{ 0 };
	descVertexBuffer.ByteWidth = sizeof(vertices);
	descVertexBuffer.Usage = D3D10_USAGE_DYNAMIC;
	descVertexBuffer.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	descVertexBuffer.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	descVertexBuffer.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA srdVertexBuffer;
	srdVertexBuffer.pSysMem = vertices;
	srdVertexBuffer.SysMemPitch = 0;
	srdVertexBuffer.SysMemSlicePitch = 0;
	mD3D10Dev->CreateBuffer(&descVertexBuffer, &srdVertexBuffer, &mVertexBuffer);

	//////////////////////////////////////////////////////////////////////////
	// create Index Buffer
	//////////////////////////////////////////////////////////////////////////

	D3D10_BUFFER_DESC descIndexBuffer{ 0 };
	descIndexBuffer.ByteWidth = sizeof(indexes);
	descIndexBuffer.Usage = D3D10_USAGE_DYNAMIC;
	descIndexBuffer.BindFlags = D3D10_BIND_INDEX_BUFFER;
	descIndexBuffer.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	descIndexBuffer.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA srdIndexData;
	srdIndexData.pSysMem = indexes;
	srdIndexData.SysMemPitch = 0;
	srdIndexData.SysMemSlicePitch = 0;
	mD3D10Dev->CreateBuffer(&descIndexBuffer, &srdIndexData, &mIndexBuffer);
}

// Created SL-160225
void CAppMain::Destroy()
{
	// window render targets
	mIndexBuffer->Release();
	mVertexBuffer->Release();
	mTexture2DShaderResourceView->Release();
	mTexture2D->Release();
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
