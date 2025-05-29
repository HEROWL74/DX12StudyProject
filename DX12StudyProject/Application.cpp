#include <string>
#include "Application.h"
#include <vector>
//�R���X�g���N�^
Application::Application(void)
{
	hMainWnd = NULL;
}

//�f�X�g���N�^
Application::~Application(void)
{

}

//���C���E�B���h�EPROC
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	std::string str = "Hello World";
	
	switch (msg) {
	case WM_PAINT:
		//�`�揈���̊J�n
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 5, 5, str.c_str(), (int)strlen(str.c_str()));
		EndPaint(hWnd, &ps);
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_DESTROY:
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);

	}
	return 0;
}

//Window�쐬�ƕ\��
bool Application::Create(HINSTANCE hInst, int nCmdShow)
{
	WNDCLASSEX wndcex;

	ZeroMemory(&wndcex, sizeof(wndcex));
	wndcex.cbSize = sizeof(WNDCLASSEX);
	wndcex.lpfnWndProc = WndProc;
	wndcex.cbClsExtra = 0;
	wndcex.cbWndExtra = 0;
	wndcex.hInstance = hInst;
	wndcex.hIcon = NULL;
	wndcex.hCursor = NULL;
	wndcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndcex.lpszClassName = tszAplName;
	wndcex.hIconSm = NULL;

	if (!RegisterClassEx(&wndcex)) {
		return false;
	}

	HWND hwnd = CreateWindowEx(0, tszAplName, tszAplName,
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WID, WINDOW_HIG,
		NULL, NULL, hInst, NULL);

	if (hwnd == NULL)return false;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	hMainWnd = hwnd;

	return true;
}

bool Application::SystemInit(void)
{

	//�f�o�C�X�̐���
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�f�o�C�X�̐��� : S_OK\n");
#endif //_DEBUG

	//�R�}���h�L���[�̐���
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pCmdQueue));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�R�}���h�L���[5�̐��� : S_OK\n");
#endif //_DEBUG

	//DXGI�t�@�N�g���[�̍쐬
	pFactory = nullptr;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("DXGI�t�@�N�g���[�̍쐬: S_OK\n");
#endif //_DEBUG

	//�X���b�v�`�F�C���̍쐬
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = WINDOW_WID;
	swapchainDesc.Height = WINDOW_HIG;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	//�o�b�N�o�b�t�@�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	//�t���b�v��͂��݂₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//�E�B���h�E�ƃt���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	hr = pFactory->CreateSwapChainForHwnd(pCmdQueue, hMainWnd, &swapchainDesc,
		nullptr, nullptr, (IDXGISwapChain1**)&pSwapChain);
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�X���b�v�`�F�C���̐��� : S_OK\n");
#endif //_DEBUG

	//�R�}���h�A���P�[�^�𐶐�
	hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&pCmdAllocator));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�R�}���h�A���P�[�^�̐��� : S_OK\n");
#endif //_DEBUG

	hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		pCmdAllocator, nullptr, IID_PPV_ARGS(&pCmdList));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�R�}���h���X�g�̐��� : S_OK\n");
#endif //_DEBUG

	//�����_�[�^�[�Q�b�g�r���[�̍쐬
	//�f�B�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //�����_�[�^�[�Q�b�g�r���[�Ȃ̂�RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;   //�\����2��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�

	hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pRtvHeap));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�f�B�X�N���v�^�q�[�v�̐��� : S_OK\n");
#endif //_DEBUG

	//�X���b�v�`�F�C���̃������ƕR�Â���
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	pSwapChain->GetDesc(&swcDesc);

	D3D12_CPU_DESCRIPTOR_HANDLE handle =
		pRtvHeap->GetCPUDescriptorHandleForHeapStart();

	std::vector<ID3D12Resource*> backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < (int)swcDesc.BufferCount; ++idx) {
		hr = pSwapChain->GetBuffer(idx, IID_PPV_ARGS(&backBuffers[idx]));
		if (FAILED(hr)) return false;
#ifdef _DEBUG
		else OutputDebugString("GetBuffer() : S_OK\n");
#endif //_DEBUG

		pDevice->CreateRenderTargetView(backBuffers[idx], nullptr, handle);

		handle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//�t�F���X�̍쐬
	pFence = nullptr;
	hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("�t�F���X�̐��� : S_OK\n");
#endif //_DEBUG

	//�C�x���g�̍쐬
	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)return false;
#ifdef _DEBUG
	else OutputDebugString("�C�x���g�̐��� : S_OK\n");
#endif //_DEBUG

	//�R�}���h���X�g�����
	pCmdList->Close();


	return true;
}

MSG Application::Run(void)
{
	//���C���̃��b�Z�[�W���[�v
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Render();
		}
	}

	return msg;
}

void Application::Render(void)
{
	//�R�}���h�A���P�[�^�̃��Z�b�g
	pCmdAllocator->Reset();
	pCmdList->Reset(pCmdAllocator, nullptr);

	//�����_�[�^�[�Q�b�g�̐ݒ�
	auto bbIdx = pSwapChain->GetCurrentBackBufferIndex();
	auto rtvH = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	pCmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	//��ʃN���A
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	pCmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);//���F

	//���߂̃N���[�Y
	pCmdList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdLists[] = { pCmdList };
	pCmdQueue->ExecuteCommandLists(1, cmdLists);

	//��ʂ̃t���b�v
	pSwapChain->Present(1, 0);
}

bool Application::Release(void)
{
	return true;
}