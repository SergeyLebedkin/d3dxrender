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

	//////////////////////////////////////////////////////////////////////////
	// create D3D12 Device and Command Queue
	//////////////////////////////////////////////////////////////////////////

	// create Direct3D 12.1 Device
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
	HRESULT hr = D3D12CreateDevice(NULL, featureLevel, __uuidof(ID3D12Device), (void**)&mD3D12Dev);

	// create Command Queue
	D3D12_COMMAND_QUEUE_DESC descCommandQueue;
	descCommandQueue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	descCommandQueue.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	descCommandQueue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	descCommandQueue.NodeMask = 0;
	hr = mD3D12Dev->CreateCommandQueue(&descCommandQueue, __uuidof(ID3D12CommandQueue), (void**)&mÑommandQueue);

	//////////////////////////////////////////////////////////////////////////
	// create Swap Chain
	//////////////////////////////////////////////////////////////////////////

	// create Swap Chain
	IDXGIFactory4* dxgiFactory = nullptr;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&dxgiFactory);
	DXGI_SWAP_CHAIN_DESC descSwapChain{ 0 };
	descSwapChain.BufferCount = 2;
	descSwapChain.BufferDesc.Height = mViewportHeight;
	descSwapChain.BufferDesc.Width = mViewportWidth;
	descSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	descSwapChain.OutputWindow = hWnd;
	descSwapChain.Windowed = true;
	descSwapChain.BufferDesc.RefreshRate.Numerator = 60;
	descSwapChain.BufferDesc.RefreshRate.Denominator = 1;
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.SampleDesc.Quality = 0;
	descSwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	descSwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	descSwapChain.Flags = 0;
	IDXGISwapChain* tempSwapChain = nullptr;
	hr = dxgiFactory->CreateSwapChain(mÑommandQueue, &descSwapChain, &tempSwapChain);
	hr = tempSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&mDXGISwapChain);
	dxgiFactory->Release();

	//////////////////////////////////////////////////////////////////////////
	// get and allocate back buffer render targets and views
	//////////////////////////////////////////////////////////////////////////

	// Get a pointer to the first back buffer from the swap chain.
	hr = mDXGISwapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&mBackBufferRenderTargets[0]);
	hr = mDXGISwapChain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)&mBackBufferRenderTargets[1]);

	// Initialize the render target view heap description for the two back buffers.
	D3D12_DESCRIPTOR_HEAP_DESC descDescriptorHeap;
	descDescriptorHeap.NumDescriptors = 2;
	descDescriptorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descDescriptorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descDescriptorHeap.NodeMask = 0;
	hr = mD3D12Dev->CreateDescriptorHeap(&descDescriptorHeap, __uuidof(ID3D12DescriptorHeap), (void**)&mRenderTargetViewHeap);

	// Create a render target view for the first back buffer.
	mBackBufferRenderTargetViews[0] = mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	mD3D12Dev->CreateRenderTargetView(mBackBufferRenderTargets[0], NULL, mBackBufferRenderTargetViews[0]);

	// Create a render target view for the second back buffer.
	mBackBufferRenderTargetViews[1] = mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	mBackBufferRenderTargetViews[1].ptr += mD3D12Dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mD3D12Dev->CreateRenderTargetView(mBackBufferRenderTargets[1], NULL, mBackBufferRenderTargetViews[1]);

	//////////////////////////////////////////////////////////////////////////
	// create Command Allocator and Command List
	//////////////////////////////////////////////////////////////////////////

	// Create a command allocator.
	hr = mD3D12Dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&mCommandAllocator);
	hr = mD3D12Dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&mGraphicsCommandList);
	mGraphicsCommandList->Close();

	//////////////////////////////////////////////////////////////////////////
	// create Fence
	//////////////////////////////////////////////////////////////////////////

	// Create a fence for GPU synchronization.
	hr = mD3D12Dev->CreateFence(mFenceValue++, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&mFence);
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

// Created SL-160225
void CAppMain::Destroy()
{
	CloseHandle(mFenceEvent);
	mFence->Release();
	mGraphicsCommandList->Release();
	mCommandAllocator->Release();
	mBackBufferRenderTargets[1]->Release();
	mBackBufferRenderTargets[0]->Release();
	mRenderTargetViewHeap->Release();
	mÑommandQueue->Release();
	mD3D12Dev->Release();
}

// Created SL-160225
void CAppMain::Render()
{
	mCurrentBackBufferIndex = mDXGISwapChain->GetCurrentBackBufferIndex();

	//////////////////////////////////////////////////////////////////////////

	// create command list
	mCommandAllocator->Reset();
	mGraphicsCommandList->Reset(mCommandAllocator, nullptr);
	
	/*
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mBackBufferRenderTargets[mCurrentBackBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	mGraphicsCommandList->ResourceBarrier(1, &barrier);
	*/

	// Then set the color to clear the window to.
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	mGraphicsCommandList->ClearRenderTargetView(mBackBufferRenderTargetViews[mCurrentBackBufferIndex], clearColor, 0, NULL);
	mGraphicsCommandList->OMSetRenderTargets(1, &mBackBufferRenderTargetViews[mCurrentBackBufferIndex], FALSE, NULL);

	/*
	// Indicate that the back buffer will now be used to present.
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mBackBufferRenderTargets[mCurrentBackBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	mGraphicsCommandList->ResourceBarrier(1, &barrier);
	*/

	mGraphicsCommandList->Close();

	//////////////////////////////////////////////////////////////////////////

	// execute command lists
	ID3D12CommandList* ppCommandLists[] = { mGraphicsCommandList };
	mÑommandQueue->ExecuteCommandLists(1, ppCommandLists);
	mÑommandQueue->Signal(mFence, mFenceValue);

	// wait for frame ends
	if (mFence->GetCompletedValue() < mFenceValue)
	{
		mFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
		mFenceValue++;
	}

	// present
	UINT buffIndex0 = mDXGISwapChain->GetCurrentBackBufferIndex();
	mDXGISwapChain->Present(0, 0);
	UINT buffIndex1 = mDXGISwapChain->GetCurrentBackBufferIndex();
}

// Created SL-160225
void CAppMain::Update(float deltaTime)
{
	// mat world
	static float angle = 0.0f;
	DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationZ(angle += deltaTime);
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
