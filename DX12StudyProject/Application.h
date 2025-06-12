#pragma once
#include <Windows.h>
#include <WinUser.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <cstdint>
#include <DirectXMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};

struct alignas(256) Transform
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
};

template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	D3D12_CPU_DESCRIPTOR_HANDLE handleCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGpu;
	T* pBuffer;
};

class Application
{
public:
	static constexpr int WINDOW_WID = 800;
	static constexpr int WINDOW_HIG = 600;

	static constexpr wchar_t tszAplName[] = L"step01";

	Application(void);
	~Application(void);

	bool Create(HINSTANCE hInst, int nCmdShow);
	bool SystemInit(void);
	MSG Run(void);
	void Render(void);
	void OnInit(void);
	bool CreateVertexBuffer(void);
	bool CreateConstantBuffer(void);
	bool CreateRootSignature(void);
	bool CreatePipelineState(void);
	bool Release(void);

private:
	static const uint32_t FRAME_BUFF_COUNT = 2;

	HWND hMainWnd;
	uint32_t mWidth;
	uint32_t mHeight;

	ID3D12Device* pDevice;
	IDXGIFactory6* pFactory;
	ID3D12CommandQueue* pCmdQueue;
	IDXGISwapChain4* pSwapChain;
	ID3D12Resource* pColorBuffer[FRAME_BUFF_COUNT];
	ID3D12CommandAllocator* pCmdAllocator[FRAME_BUFF_COUNT];
	ID3D12GraphicsCommandList* pCmdList;
	ID3D12DescriptorHeap* pRtvHeap;
	ID3D12Fence* pFence;
	ID3D12DescriptorHeap* pHeapCBV;

	ID3D12Resource* pVB;
	ID3D12Resource* pCB[FRAME_BUFF_COUNT];
	ID3D12RootSignature* pRootSignature;
	ID3D12PipelineState* pPSO;

	HANDLE fenceEvent;

	uint64_t fenceCounter[FRAME_BUFF_COUNT];
	uint32_t m_FrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV[FRAME_BUFF_COUNT];

	D3D12_VERTEX_BUFFER_VIEW vertexBV;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Scissor;
	ConstantBufferView<Transform> m_CBV[FRAME_BUFF_COUNT];

	float m_RotateAngle;
};