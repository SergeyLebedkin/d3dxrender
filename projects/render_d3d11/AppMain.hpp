#pragma once

#pragma warning( push )
#pragma warning( disable : 4005)
#include <D3D11.h>
#include <D3DX11.h>
#pragma warning( pop )
#include <DirectXMath.h>

// created SL-160225
class CAppMain
{
private:
	// window parameters
	WORD mViewportWidth = 0;
	WORD mViewportHeight = 0;

	// DXGI handlers
	IDXGISwapChain*         mDXGISwapChain = nullptr;

	// D3D11 handlers
	ID3D11Device*        mD3D11Dev = nullptr;
	ID3D11DeviceContext* mD3D11DevCtx = nullptr;

	// window render targets
	ID3D11Texture2D*        mWindowRenderTargetTexture2D = nullptr;
	ID3D11RenderTargetView* mWindowRenderTargetView = nullptr;
	ID3D11Texture2D*        mWindowDepthStencilTexture2D = nullptr;
	ID3D11DepthStencilView* mWindowDepthStencilView = nullptr;

	// D3D11 scene handles
	//ID3D11Texture2D*          mTexture2D = nullptr;
	//ID3D11ShaderResourceView* mTexture2DShaderResourceView = nullptr;
	//ID3D11RenderTargetView*   mTexture2DRenderTargetView = nullptr;
	ID3D11Texture2D*          mTexture2DFromFile = nullptr;
	ID3D11ShaderResourceView* mTexture2DShaderResourceViewFromFile = nullptr;
	ID3D11SamplerState*       mSamplerState = nullptr;
	ID3D11Buffer*             mVertexBuffer = nullptr;
	ID3D11Buffer*             mIndexBuffer = nullptr;
	ID3D11Buffer*             mConstantBuffer = nullptr;
	ID3D11VertexShader*       mVertexShader = nullptr;
	ID3D11PixelShader*        mPixelShader = nullptr;
	ID3D11InputLayout*        mInputLayout = nullptr;
	ID3D11RasterizerState*    mRasterizerState = nullptr;
	ID3D11BlendState*         mBlendState = nullptr;
	ID3D11DepthStencilState*  mDepthStencilState = nullptr;

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