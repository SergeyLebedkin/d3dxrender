#include "AppMain.hpp"
#include <iostream>
#include <fstream>
#include <string>

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
}

// Created SL-160225
void CAppMain::Destroy()
{
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
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};