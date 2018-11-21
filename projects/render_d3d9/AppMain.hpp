#pragma once

#include <d3d9.h>
#include <d3dx9.h>

// created SL-160225
class CAppMain
{
private:
	// window parameters
	WORD mViewportWidth = 0;
	WORD mViewportHeight = 0;

	// direct3d and device
	IDirect3D9*       mD3D9 = nullptr;
	IDirect3DDevice9* mD3D9Dev = nullptr;

	// handles
	IDirect3DTexture9*           mTexture0 = nullptr;
	IDirect3DVertexBuffer9*      mVertexBuffer = nullptr;
	IDirect3DIndexBuffer9*       mIndexBuffer = nullptr;
	IDirect3DVertexDeclaration9* mVertexDeclaration = nullptr;
	IDirect3DVertexShader9*      mVertexShader = nullptr;
	IDirect3DPixelShader9*       mPixelShader = nullptr;
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