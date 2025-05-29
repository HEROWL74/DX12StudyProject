#include <string>
#include "Application.h"
#include <vector>
//コンストラクタ
Application::Application(void)
{
	hMainWnd = NULL;
}

//デストラクタ
Application::~Application(void)
{

}

//メインウィンドウPROC
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	std::string str = "Hello World";
	
	switch (msg) {
	case WM_PAINT:
		//描画処理の開始
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

//Window作成と表示
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

	//デバイスの生成
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("デバイスの生成 : S_OK\n");
#endif //_DEBUG

	//コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pCmdQueue));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("コマンドキュー5の生成 : S_OK\n");
#endif //_DEBUG

	//DXGIファクトリーの作成
	pFactory = nullptr;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("DXGIファクトリーの作成: S_OK\n");
#endif //_DEBUG

	//スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = WINDOW_WID;
	swapchainDesc.Height = WINDOW_HIG;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	//バックバッファは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	//フリップ後はすみやかに破棄
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//特に指定なし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//ウィンドウとフルスクリーン切り替え可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	hr = pFactory->CreateSwapChainForHwnd(pCmdQueue, hMainWnd, &swapchainDesc,
		nullptr, nullptr, (IDXGISwapChain1**)&pSwapChain);
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("スワップチェインの生成 : S_OK\n");
#endif //_DEBUG

	//コマンドアロケータを生成
	hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&pCmdAllocator));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("コマンドアロケータの生成 : S_OK\n");
#endif //_DEBUG

	hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		pCmdAllocator, nullptr, IID_PPV_ARGS(&pCmdList));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("コマンドリストの生成 : S_OK\n");
#endif //_DEBUG

	//レンダーターゲットビューの作成
	//ディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビューなのでRTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;   //表裏の2つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし

	hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pRtvHeap));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("ディスクリプタヒープの生成 : S_OK\n");
#endif //_DEBUG

	//スワップチェインのメモリと紐づける
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

	//フェンスの作成
	pFence = nullptr;
	hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
	if (FAILED(hr)) return false;
#ifdef _DEBUG
	else OutputDebugString("フェンスの生成 : S_OK\n");
#endif //_DEBUG

	//イベントの作成
	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)return false;
#ifdef _DEBUG
	else OutputDebugString("イベントの生成 : S_OK\n");
#endif //_DEBUG

	//コマンドリストを閉じる
	pCmdList->Close();


	return true;
}

MSG Application::Run(void)
{
	//メインのメッセージループ
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
	//コマンドアロケータのリセット
	pCmdAllocator->Reset();
	pCmdList->Reset(pCmdAllocator, nullptr);

	//レンダーターゲットの設定
	auto bbIdx = pSwapChain->GetCurrentBackBufferIndex();
	auto rtvH = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	pCmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	//画面クリア
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	pCmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);//黄色

	//命令のクローズ
	pCmdList->Close();

	//コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { pCmdList };
	pCmdQueue->ExecuteCommandLists(1, cmdLists);

	//画面のフリップ
	pSwapChain->Present(1, 0);
}

bool Application::Release(void)
{
	return true;
}