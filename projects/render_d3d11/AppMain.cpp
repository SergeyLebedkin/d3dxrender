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

	// create swap chain description
	DXGI_SWAP_CHAIN_DESC descSwapChain{ 0 };
	descSwapChain.BufferCount = 1;
	descSwapChain.BufferDesc.Height = mViewportHeight;
	descSwapChain.BufferDesc.Width = mViewportWidth;
	descSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	descSwapChain.OutputWindow = hWnd;
	descSwapChain.Windowed = true;
	descSwapChain.BufferDesc.RefreshRate.Numerator = 0;
	descSwapChain.BufferDesc.RefreshRate.Denominator = 1;
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.SampleDesc.Quality = 0;
	descSwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	descSwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	descSwapChain.Flags = 0;
	D3D_FEATURE_LEVEL pFeatureLevels[] = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
	UINT pFeatureLevelsCount = sizeof(pFeatureLevels) / sizeof(pFeatureLevels[0]);
	D3D_FEATURE_LEVEL pFeatureLevelsSelected = D3D_FEATURE_LEVEL_11_0;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, pFeatureLevels, pFeatureLevelsCount, D3D11_SDK_VERSION, &descSwapChain, &mDXGISwapChain, &mD3D11Dev, &pFeatureLevelsSelected, &mD3D11DevCtx);

	// get mWindowRenderTargetTexture2D and create mWindowRenderTargetView
	mDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mWindowRenderTargetTexture2D);
	hr = mD3D11Dev->CreateRenderTargetView(mWindowRenderTargetTexture2D, NULL, &mWindowRenderTargetView);
	D3D11_TEXTURE2D_DESC descRTT;
	mWindowRenderTargetTexture2D->GetDesc(&descRTT);

	//////////////////////////////////////////////////////////////////////////
	// create Window Depth Stencil Texture2D and Window Depth Stencil View
	//////////////////////////////////////////////////////////////////////////

	// create Window Depth Stencil Texture2D
	D3D11_TEXTURE2D_DESC descDST{ 0 };
	descDST.Width = descRTT.Width;
	descDST.Height = descRTT.Height;
	descDST.MipLevels = descRTT.MipLevels;
	descDST.ArraySize = descRTT.ArraySize;
	descDST.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDST.SampleDesc.Count = 1;
	descDST.SampleDesc.Quality = 0;
	descDST.Usage = D3D11_USAGE_DEFAULT;
	descDST.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDST.CPUAccessFlags = 0;
	descDST.MiscFlags = 0;
	hr = mD3D11Dev->CreateTexture2D(&descDST, nullptr, &mWindowDepthStencilTexture2D);

	// create Window Depth Stencil View
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDST.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	hr = mD3D11Dev->CreateDepthStencilView(mWindowDepthStencilTexture2D, &descDSV, &mWindowDepthStencilView);

	//////////////////////////////////////////////////////////////////////////
	// create Texture2D and Texture2DShaderResourceView from file
	//////////////////////////////////////////////////////////////////////////

	// load texture from file
	D3D11_TEXTURE2D_DESC descTex2D;
	ID3D11Resource* pD3D11Resource = NULL;
	hr = D3DX11CreateTextureFromFile(mD3D11Dev, L"textures/texture.png", NULL, NULL, &pD3D11Resource, NULL);
	pD3D11Resource->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&mTexture2DFromFile);
	mTexture2DFromFile->GetDesc(&descTex2D);
	pD3D11Resource->Release();

	// create Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	descSRV.Format = descTex2D.Format;
	descSRV.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = descTex2D.MipLevels;
	descSRV.Texture2D.MostDetailedMip = 0;
	hr = mD3D11Dev->CreateShaderResourceView(mTexture2DFromFile, &descSRV, &mTexture2DShaderResourceViewFromFile);

	//////////////////////////////////////////////////////////////////////////
	// create Sampler
	//////////////////////////////////////////////////////////////////////////

	D3D11_SAMPLER_DESC descSampler;
	descSampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	descSampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	descSampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	descSampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	descSampler.MipLODBias = 0.0f;
	descSampler.MaxAnisotropy = 16;
	descSampler.ComparisonFunc = D3D11_COMPARISON_NEVER;
	descSampler.BorderColor[0] = 1.0f;
	descSampler.BorderColor[1] = 0.0f;
	descSampler.BorderColor[2] = 0.0f;
	descSampler.BorderColor[3] = 1.0f;
	descSampler.MinLOD = -FLT_MAX;
	descSampler.MaxLOD = FLT_MAX;
	hr = mD3D11Dev->CreateSamplerState(&descSampler, &mSamplerState);

	//////////////////////////////////////////////////////////////////////////
	// create Vertex Buffer
	//////////////////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC descVertexBuffer{ 0 };
	descVertexBuffer.ByteWidth = sizeof(vertices);
	descVertexBuffer.Usage = D3D11_USAGE_DYNAMIC;
	descVertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	descVertexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descVertexBuffer.MiscFlags = 0;
	descVertexBuffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA srdVertexBuffer;
	srdVertexBuffer.pSysMem = vertices;
	srdVertexBuffer.SysMemPitch = 0;
	srdVertexBuffer.SysMemSlicePitch = 0;
	hr = mD3D11Dev->CreateBuffer(&descVertexBuffer, &srdVertexBuffer, &mVertexBuffer);

	//////////////////////////////////////////////////////////////////////////
	// create Index Buffer
	//////////////////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC descIndexBuffer{ 0 };
	descIndexBuffer.ByteWidth = sizeof(indexes);
	descIndexBuffer.Usage = D3D11_USAGE_DYNAMIC;
	descIndexBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	descIndexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descIndexBuffer.MiscFlags = 0;
	descIndexBuffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA srdIndexBuffer;
	srdIndexBuffer.pSysMem = indexes;
	srdIndexBuffer.SysMemPitch = 0;
	srdIndexBuffer.SysMemSlicePitch = 0;
	hr = mD3D11Dev->CreateBuffer(&descIndexBuffer, &srdIndexBuffer, &mIndexBuffer);

	//////////////////////////////////////////////////////////////////////////
	// create Constant Buffer
	//////////////////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC descConstantBuffer{ 0 };
	descConstantBuffer.ByteWidth = sizeof(DirectX::XMMATRIX);
	descConstantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	descConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	descConstantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descConstantBuffer.MiscFlags = 0;
	descIndexBuffer.StructureByteStride = 0;
	D3D10_SUBRESOURCE_DATA srdConstantBuffer;
	srdConstantBuffer.pSysMem = NULL;
	srdConstantBuffer.SysMemPitch = 0;
	srdConstantBuffer.SysMemSlicePitch = 0;
	hr = mD3D11Dev->CreateBuffer(&descConstantBuffer, NULL, &mConstantBuffer);

	//////////////////////////////////////////////////////////////////////////
	// create Vertex Shader
	//////////////////////////////////////////////////////////////////////////

	ID3D10Blob *pVertexShaderBlob = nullptr;
	ID3D10Blob *pVertexShaderErrorBlob = nullptr;
	if (FAILED(D3DX11CompileFromFile(L"shaders/vs.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, NULL, &pVertexShaderBlob, &pVertexShaderErrorBlob, NULL))) throw 1;
	hr = mD3D11Dev->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), NULL, &mVertexShader);

	//////////////////////////////////////////////////////////////////////////
	// create Pixel Shader
	//////////////////////////////////////////////////////////////////////////

	ID3D10Blob *pPixelShaderBlob = nullptr;
	ID3D10Blob *pPixelShaderErrorBlob = nullptr;
	if (FAILED(D3DX11CompileFromFile(L"shaders/ps.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, NULL, &pPixelShaderBlob, &pPixelShaderErrorBlob, NULL))) throw 2;
	hr = mD3D11Dev->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), NULL, &mPixelShader);
	if (pPixelShaderErrorBlob) pPixelShaderErrorBlob->Release();
	if (pPixelShaderBlob) pPixelShaderBlob->Release();

	//////////////////////////////////////////////////////////////////////////
	// create Input Layout
	//////////////////////////////////////////////////////////////////////////

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT      , 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = mD3D11Dev->CreateInputLayout(layout, sizeof(layout) / sizeof(layout[0]), pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), &mInputLayout);
	if (pVertexShaderErrorBlob) pVertexShaderErrorBlob->Release();
	if (pVertexShaderBlob) pVertexShaderBlob->Release();

	//////////////////////////////////////////////////////////////////////////
	// create Rasterizer State
	//////////////////////////////////////////////////////////////////////////

	D3D11_RASTERIZER_DESC descRasterizer;
	descRasterizer.CullMode = D3D11_CULL_NONE;
	descRasterizer.FillMode = D3D11_FILL_SOLID;
	descRasterizer.FrontCounterClockwise = true;
	descRasterizer.DepthBias = false;
	descRasterizer.DepthBiasClamp = 0;
	descRasterizer.SlopeScaledDepthBias = 0;
	descRasterizer.DepthClipEnable = true;
	descRasterizer.ScissorEnable = false;
	descRasterizer.MultisampleEnable = false;
	descRasterizer.AntialiasedLineEnable = true;
	hr = mD3D11Dev->CreateRasterizerState(&descRasterizer, &mRasterizerState);

	//////////////////////////////////////////////////////////////////////////
	// create Blend State
	//////////////////////////////////////////////////////////////////////////

	D3D11_BLEND_DESC descBlend{ 0 };
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	descBlend.RenderTarget[0].BlendEnable = FALSE;
	descBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	descBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	descBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	descBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	descBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	descBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	descBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = mD3D11Dev->CreateBlendState(&descBlend, &mBlendState);

	//////////////////////////////////////////////////////////////////////////
	// create Depth Stencil State
	//////////////////////////////////////////////////////////////////////////

	D3D11_DEPTH_STENCIL_DESC descDepthStencil{ 0 };
	descDepthStencil.DepthEnable = true;
	descDepthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepthStencil.DepthFunc = D3D11_COMPARISON_LESS;
	descDepthStencil.StencilEnable = false;
	descDepthStencil.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepthStencil.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	descDepthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	descDepthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	descDepthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	descDepthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	descDepthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	descDepthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	descDepthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	descDepthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = mD3D11Dev->CreateDepthStencilState(&descDepthStencil, &mDepthStencilState);
}

// Created SL-160225
void CAppMain::Destroy()
{
	// window render targets
	mDepthStencilState->Release();
	mBlendState->Release();
	mRasterizerState->Release();
	mInputLayout->Release();
	mPixelShader->Release();
	mVertexShader->Release();
	mConstantBuffer->Release();
	mIndexBuffer->Release();
	mVertexBuffer->Release();
	mSamplerState->Release();
	mTexture2DShaderResourceViewFromFile->Release();
	mTexture2DFromFile->Release();

	// texture
	//mTexture2DShaderResourceView->Release();
	//mTexture2DRenderTargetView->Release();
	//mTexture2D->Release();

	// window render targets
	mWindowDepthStencilView->Release();
	mWindowDepthStencilTexture2D->Release();
	mWindowRenderTargetView->Release();
	mWindowRenderTargetTexture2D->Release();

	// D3D10 handlers
	mD3D11DevCtx->Release();
	mD3D11Dev->Release();

	// DXGI handlers
	mDXGISwapChain->Release();
}

// Created SL-160225
void CAppMain::Render()
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = mViewportWidth;
	vp.Height = mViewportHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	// Clear the back buffer 
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	mD3D11DevCtx->ClearRenderTargetView(mWindowRenderTargetView, clearColor);
	mD3D11DevCtx->ClearDepthStencilView(mWindowDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

	// Input-Assembler Stage
	UINT stride = sizeof(CUSTOMVERTEX);
	UINT offset = 0;
	mD3D11DevCtx->IASetInputLayout(mInputLayout);
	mD3D11DevCtx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mD3D11DevCtx->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mD3D11DevCtx->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// update constant buffer data
	D3D11_MAPPED_SUBRESOURCE ms;
	mD3D11DevCtx->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, &mWVP, sizeof(mWVP));
	mD3D11DevCtx->Unmap(mConstantBuffer, NULL);

	// Vertex-Shader Stage
	mD3D11DevCtx->VSSetConstantBuffers(0, 1, &mConstantBuffer);
	mD3D11DevCtx->VSSetSamplers(0, 0, NULL);
	mD3D11DevCtx->VSSetShader(mVertexShader, NULL, 0);
	mD3D11DevCtx->VSSetShaderResources(0, 0, NULL);

	// Hull-Shader Stage
	mD3D11DevCtx->HSSetConstantBuffers(0, 0, NULL);
	mD3D11DevCtx->HSSetSamplers(0, 0, NULL);
	mD3D11DevCtx->HSSetShader(NULL, NULL, 0);
	mD3D11DevCtx->HSSetShaderResources(0, 0, NULL);

	// Domain-Shader Stage
	mD3D11DevCtx->DSSetConstantBuffers(0, 0, NULL);
	mD3D11DevCtx->DSSetSamplers(0, 0, NULL);
	mD3D11DevCtx->DSSetShader(NULL, NULL, 0);
	mD3D11DevCtx->DSSetShaderResources(0, 0, NULL);

	// Geometry-Shader Stage
	mD3D11DevCtx->GSSetConstantBuffers(0, 0, NULL);
	mD3D11DevCtx->GSSetSamplers(0, 0, NULL);
	mD3D11DevCtx->GSSetShader(NULL, NULL, 0);
	mD3D11DevCtx->GSSetShaderResources(0, 0, NULL);

	// Stream-Output Stage
	mD3D11DevCtx->SOSetTargets(0, NULL, NULL);

	// Rasterizer Stage
	mD3D11DevCtx->RSSetScissorRects(0, NULL);
	mD3D11DevCtx->RSSetState(mRasterizerState);
	mD3D11DevCtx->RSSetViewports(1, &vp);

	// Geometry-Shader Stage
	mD3D11DevCtx->PSSetConstantBuffers(0, 0, NULL);
	mD3D11DevCtx->PSSetSamplers(0, 1, &mSamplerState);
	mD3D11DevCtx->PSSetShader(mPixelShader, NULL, 0);
	mD3D11DevCtx->PSSetShaderResources(0, 1, &mTexture2DShaderResourceViewFromFile);

	// set render target view
	mD3D11DevCtx->OMSetBlendState(mBlendState, NULL, 0xffffffff);
	mD3D11DevCtx->OMSetDepthStencilState(mDepthStencilState, 0);
	mD3D11DevCtx->OMSetRenderTargets(1, &mWindowRenderTargetView, mWindowDepthStencilView);

	// DRAW !!!!!
	mD3D11DevCtx->DrawIndexed(6, 0, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	mDXGISwapChain->Present(0, 0);
}

// Created SL-160225
void CAppMain::Update(float deltaTime)
{
	// mat world
	DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationZ(0.0f);
	DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX matTranslate = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX matWorld = matRotate * matScale * matTranslate;

	// mat view
	DirectX::XMMATRIX matView = DirectX::XMMatrixLookAtRH(
		DirectX::XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f), // the camera position
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),  // the look-at position
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)   // the up direction
	);

	// mat projection
	DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(45.0f), (FLOAT)mViewportWidth / mViewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	mWVP = matWorld * matView * matProj;
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};
