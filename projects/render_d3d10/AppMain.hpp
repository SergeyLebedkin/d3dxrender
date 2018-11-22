#pragma once

#pragma warning( push )
#pragma warning( disable : 4005)
#include <D3D10.h>
#include <D3DX10.h>
#pragma warning( pop )

// created SL-160225
class CAppMain
{
private:
	// window parameters
	WORD mViewportWidth = 0;
	WORD mViewportHeight = 0;

	// DXGI handlers
	IDXGISwapChain*         mDXGISwapChain = nullptr;

	// D3D10 handlers
	ID3D10Device*           mD3D10Dev = nullptr;

	// window render targets
	ID3D10Texture2D*        mWindowRenderTargetTexture2D = nullptr;
	ID3D10RenderTargetView* mWindowRenderTargetView = nullptr;
	ID3D10Texture2D*        mWindowDepthStencilTexture2D = nullptr;
	ID3D10DepthStencilView* mWindowDepthStencilView = nullptr;

	// D3D10 scene handles
	ID3D10Texture2D*          mTexture2D = nullptr;
	ID3D10ShaderResourceView* mTexture2DShaderResourceView = nullptr;
	ID3D10Buffer*             mVertexBuffer = nullptr;
	ID3D10Buffer*             mIndexBuffer = nullptr;
public:
	CAppMain() {};
	virtual ~CAppMain() {};

	// main functions
	void Init(const HWND hWnd);
	void Destroy();
	void Render();

	// SetViewportSize
	void SetViewportSize(WORD viewportWidth, WORD viewportHeight);
};