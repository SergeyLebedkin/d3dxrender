#pragma once

#pragma warning( push )
#pragma warning( disable : 4005)
#include <D3D11.h>
#include <D3DX10.h>
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

	// D3D10 handlers
	ID3D11Device* mD3D11Dev = nullptr;
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