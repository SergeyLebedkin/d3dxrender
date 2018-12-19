#pragma once

#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// created SL-160225
class CAppMain
{
private:
	// window parameters
	WORD mViewportWidth = 0;
	WORD mViewportHeight = 0;

	// DXGI handlers
	IDXGISwapChain3* mDXGISwapChain = nullptr;

	// D3D12 handlers
	ID3D12Device*       mD3D12Dev = nullptr;
	ID3D12CommandQueue* m—ommandQueue = nullptr;

	// Handlers
	ID3D12DescriptorHeap*       mRenderTargetViewHeap = nullptr;
	ID3D12Resource*             mBackBufferRenderTargets[2]{ nullptr, nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE mBackBufferRenderTargetViews[2];
	ID3D12CommandAllocator*     mCommandAllocator = nullptr;
	ID3D12GraphicsCommandList*  mGraphicsCommandList = nullptr;
	ID3D12Fence*                mFence = nullptr;
	ID3D12PipelineState*        mPipelineState = nullptr;

	// events and fences
	WORD                        mCurrentBackBufferIndex = 0;
	HANDLE                      mFenceEvent = nullptr; 
	UINT64                      mFenceValue = 0;

	// scene variables
	DirectX::XMMATRIX mWVP;
public:
	CAppMain() {};
	virtual ~CAppMain() {};

	// main functions
	void Init(const HWND hWnd);
	void Destroy(); 
	void Render();
	void Update(float deltaTime);

	// SetViewportSize
	void SetViewportSize(WORD viewportWidth, WORD viewportHeight);
};