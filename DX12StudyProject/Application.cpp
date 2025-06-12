#include <string>
#include "Application.h"
#include <vector>

Application::Application(void)
{
	hMainWnd = NULL;
	pDevice = nullptr;
	pFactory = nullptr;
	pCmdQueue = nullptr;
	pSwapChain = nullptr;
	pCmdList = nullptr;
	pRtvHeap = nullptr;
	pFence = nullptr;
	pHeapCBV = nullptr;
	pVB = nullptr;
	pRootSignature = nullptr;
	pPSO = nullptr;
	fenceEvent = nullptr;
	m_FrameIndex = 0;
	m_RotateAngle = 0.0f;
	mWidth = WINDOW_WID;
	mHeight = WINDOW_HIG;

	for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
		pColorBuffer[i] = nullptr;
		pCmdAllocator[i] = nullptr;
		pCB[i] = nullptr;
		fenceCounter[i] = 0;
	}
}

Application::~Application(void)
{
	Release();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	std::wstring str = L"Hello World";

	switch (msg) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 5, 5, str.c_str(), (int)str.length());
		EndPaint(hWnd, &ps);
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

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

	if (hwnd == NULL) return false;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	hMainWnd = hwnd;

	return true;
}

bool Application::SystemInit(void)
{
	// デバイスの生成
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed to create D3D12 device\n");
		return false;
	}
	OutputDebugString(L"D3D12 device created successfully\n");

	// コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pCmdQueue));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed to create command queue\n");
		return false;
	}
	OutputDebugString(L"Command queue created successfully\n");

	// DXGIファクトリーの作成
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr)) return false;

	// スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = WINDOW_WID;
	swapchainDesc.Height = WINDOW_HIG;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = FRAME_BUFF_COUNT;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = pFactory->CreateSwapChainForHwnd(pCmdQueue, hMainWnd, &swapchainDesc,
		nullptr, nullptr, (IDXGISwapChain1**)&pSwapChain);
	if (FAILED(hr)) return false;

	// コマンドアロケーターを生成
	for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
		hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&pCmdAllocator[i]));
		if (FAILED(hr)) return false;
	}

	hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		pCmdAllocator[0], nullptr, IID_PPV_ARGS(&pCmdList));
	if (FAILED(hr)) return false;

	// レンダーターゲットビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = FRAME_BUFF_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pRtvHeap));
	if (FAILED(hr)) return false;

	// CBV用のデスクリプタヒープを作成
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.NodeMask = 0;
	cbvHeapDesc.NumDescriptors = FRAME_BUFF_COUNT;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = pDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&pHeapCBV));
	if (FAILED(hr)) return false;

	// スワップチェインのメモリと結び付ける
	D3D12_CPU_DESCRIPTOR_HANDLE handle = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto handleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int idx = 0; idx < FRAME_BUFF_COUNT; ++idx) {
		hr = pSwapChain->GetBuffer(idx, IID_PPV_ARGS(&pColorBuffer[idx]));
		if (FAILED(hr)) return false;

		pDevice->CreateRenderTargetView(pColorBuffer[idx], nullptr, handle);
		handleRTV[idx] = handle;
		handle.ptr += handleSize;
	}

	// フェンスの作成
	hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
	if (FAILED(hr)) return false;

	// イベントの作成
	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr) return false;

	// ビューポートとシザー矩形の設定
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = static_cast<float>(WINDOW_WID);
	m_Viewport.Height = static_cast<float>(WINDOW_HIG);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_Scissor.left = 0;
	m_Scissor.top = 0;
	m_Scissor.right = WINDOW_WID;
	m_Scissor.bottom = WINDOW_HIG;

	// コマンドリストを閉じる
	pCmdList->Close();

	// 初期化関数を呼び出し
	OnInit();

	return true;
}

void Application::OnInit(void)
{
	OutputDebugString(L"Starting OnInit...\n");

	// 頂点バッファの作成
	if (!CreateVertexBuffer()) {
		OutputDebugString(L"Failed to create vertex buffer\n");
		return;
	}
	OutputDebugString(L"Vertex buffer created\n");

	// 定数バッファの作成
	if (!CreateConstantBuffer()) {
		OutputDebugString(L"Failed to create constant buffer\n");
		return;
	}
	OutputDebugString(L"Constant buffer created\n");

	// ルートシグネチャの作成
	if (!CreateRootSignature()) {
		OutputDebugString(L"Failed to create root signature\n");
		return;
	}
	OutputDebugString(L"Root signature created\n");

	// パイプラインステートの作成
	if (!CreatePipelineState()) {
		OutputDebugString(L"Failed to create pipeline state\n");
		return;
	}
	OutputDebugString(L"Pipeline state created\n");

	OutputDebugString(L"OnInit completed successfully\n");
}

bool Application::CreateVertexBuffer(void)
{
	// 頂点データ（時計回りに修正）
	Vertex vertices[] = {
		{DirectX::XMFLOAT3(-0.8f,-0.8f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}, // 青（左下）
		{DirectX::XMFLOAT3(0.0f, 0.8f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}, // 赤（上）
		{DirectX::XMFLOAT3(0.8f,-0.8f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)}, // 緑（右下）
	};

	OutputDebugString(L"Creating vertex buffer with vertices (clockwise order):\n");
	for (int i = 0; i < 3; i++) {
		wchar_t buffer[256];
		swprintf_s(buffer, L"Vertex %d: Pos(%.2f, %.2f, %.2f) Color(%.2f, %.2f, %.2f, %.2f)\n",
			i, vertices[i].Position.x, vertices[i].Position.y, vertices[i].Position.z,
			vertices[i].Color.x, vertices[i].Color.y, vertices[i].Color.z, vertices[i].Color.w);
		OutputDebugString(buffer);
	}

	// ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// リソースの設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = sizeof(vertices);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 頂点バッファの作成
	auto hr = pDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pVB));

	if (FAILED(hr)) return false;

	// データのマッピング
	void* pData = nullptr;
	hr = pVB->Map(0, nullptr, &pData);
	if (FAILED(hr)) return false;

	memcpy(pData, vertices, sizeof(vertices));
	pVB->Unmap(0, nullptr);

	// 頂点バッファビューの設定
	vertexBV.BufferLocation = pVB->GetGPUVirtualAddress();
	vertexBV.SizeInBytes = sizeof(vertices);
	vertexBV.StrideInBytes = sizeof(Vertex);

	return true;
}

bool Application::CreateConstantBuffer(void)
{
	// ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// リソースの設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = sizeof(Transform);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto cbvHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
		// 定数バッファの作成
		auto hr = pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pCB[i]));

		if (FAILED(hr)) return false;

		// データのマッピング
		hr = pCB[i]->Map(0, nullptr, (void**)&m_CBV[i].pBuffer);
		if (FAILED(hr)) return false;

		// CBVの設定
		m_CBV[i].desc.BufferLocation = pCB[i]->GetGPUVirtualAddress();
		m_CBV[i].desc.SizeInBytes = sizeof(Transform);

		// CBVハンドルの設定
		m_CBV[i].handleCpu = pHeapCBV->GetCPUDescriptorHandleForHeapStart();
		m_CBV[i].handleCpu.ptr += i * cbvHandleSize;

		m_CBV[i].handleGpu = pHeapCBV->GetGPUDescriptorHandleForHeapStart();
		m_CBV[i].handleGpu.ptr += i * cbvHandleSize;

		// CBVの作成
		pDevice->CreateConstantBufferView(&m_CBV[i].desc, m_CBV[i].handleCpu);
	}

	return true;
}

bool Application::CreateRootSignature(void)
{
	// ルートパラメーターの設定
	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam.Descriptor.ShaderRegister = 0;
	rootParam.Descriptor.RegisterSpace = 0;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.NumParameters = 1;
	rootSigDesc.pParameters = &rootParam;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// シリアライズ
	ID3DBlob* pBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	auto hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pBlob, &pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) pErrorBlob->Release();
		return false;
	}

	// ルートシグネチャの作成
	hr = pDevice->CreateRootSignature(0, pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));

	if (pBlob) pBlob->Release();
	if (pErrorBlob) pErrorBlob->Release();

	return SUCCEEDED(hr);
}

bool Application::CreatePipelineState(void)
{
	// 頂点シェーダーのコンパイル
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	auto hr = D3DCompileFromFile(
		L"SimpleVS.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pVSBlob,
		&pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {
			OutputDebugString(L"VS Compile Error: ");
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			OutputDebugString(L"\n");
			pErrorBlob->Release();
		}
		return false;
	}
	OutputDebugString(L"Vertex shader compiled successfully\n");

	// ピクセルシェーダーのコンパイル
	ID3DBlob* pPSBlob = nullptr;
	hr = D3DCompileFromFile(
		L"SimplePS.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pPSBlob,
		&pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {
			OutputDebugString(L"PS Compile Error: ");
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			OutputDebugString(L"\n");
			pErrorBlob->Release();
		}
		if (pVSBlob) pVSBlob->Release();
		return false;
	}
	OutputDebugString(L"Pixel shader compiled successfully\n");

	// 入力レイアウトの設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
		  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// パイプラインステートの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = pRootSignature;
	psoDesc.VS.pShaderBytecode = pVSBlob->GetBufferPointer();
	psoDesc.VS.BytecodeLength = pVSBlob->GetBufferSize();
	psoDesc.PS.pShaderBytecode = pPSBlob->GetBufferPointer();
	psoDesc.PS.BytecodeLength = pPSBlob->GetBufferSize();
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ソリッドに戻す
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // カリングは無効のまま
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// パイプラインステートオブジェクトの作成
	hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPSO));

	if (pVSBlob) pVSBlob->Release();
	if (pPSBlob) pPSBlob->Release();

	if (FAILED(hr)) {
		OutputDebugString(L"Failed to create pipeline state\n");
		return false;
	}
	OutputDebugString(L"Pipeline state created successfully\n");

	return true;
}

MSG Application::Run(void)
{
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
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
	// フレームインデックスを取得
	m_FrameIndex = pSwapChain->GetCurrentBackBufferIndex();

	// コマンドアロケーターのリセット
	pCmdAllocator[m_FrameIndex]->Reset();
	pCmdList->Reset(pCmdAllocator[m_FrameIndex], nullptr);

	// リソースバリア（プレゼント → レンダーターゲット）
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = pColorBuffer[m_FrameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCmdList->ResourceBarrier(1, &barrier);

	// レンダーターゲットの設定
	pCmdList->OMSetRenderTargets(1, &handleRTV[m_FrameIndex], true, nullptr);

	// 画面クリア
	float clearColor[] = { 0.2f, 0.2f, 0.8f, 1.0f }; // より明るい青色に変更
	pCmdList->ClearRenderTargetView(handleRTV[m_FrameIndex], clearColor, 0, nullptr);

	// パイプラインステートとルートシグネチャが有効かチェック
	if (pPSO && pRootSignature) {
		// ビューポートとシザー矩形の設定
		pCmdList->RSSetViewports(1, &m_Viewport);
		pCmdList->RSSetScissorRects(1, &m_Scissor);

		// パイプラインステートの設定
		pCmdList->SetPipelineState(pPSO);

		// ルートシグネチャの設定
		pCmdList->SetGraphicsRootSignature(pRootSignature);

		// デスクリプタヒープの設定
		ID3D12DescriptorHeap* heaps[] = { pHeapCBV };
		pCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

		// 変換行列の更新（回転アニメーション付き）
		static bool firstFrame = true;
		if (firstFrame) {
			OutputDebugString(L"Setting up transformation matrices\n");
			firstFrame = false;
		}

		m_RotateAngle += 0.02f; // 回転角度を更新

		// ワールド変換行列（Y軸回転）
		XMMATRIX world = XMMatrixRotationY(m_RotateAngle);

		// ビュー変換行列（カメラ位置）
		XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);  // カメラを少し遠くに
		XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);

		// プロジェクション変換行列
		XMMATRIX proj = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(45.0f),                    // 視野角
			static_cast<float>(mWidth) / static_cast<float>(mHeight), // アスペクト比
			0.1f,                                         // ニアクリップ
			100.0f                                        // ファークリップ
		);

		// 定数バッファの更新
		m_CBV[m_FrameIndex].pBuffer->World = XMMatrixTranspose(world);
		m_CBV[m_FrameIndex].pBuffer->View = XMMatrixTranspose(view);
		m_CBV[m_FrameIndex].pBuffer->Proj = XMMatrixTranspose(proj);

		// 定数バッファの設定
		pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex].desc.BufferLocation);

		// プリミティブトポロジーの設定
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 頂点バッファの設定
		pCmdList->IASetVertexBuffers(0, 1, &vertexBV);

		// 描画
		static bool firstDraw = true;
		if (firstDraw) {
			OutputDebugString(L"Executing DrawInstanced command\n");
			firstDraw = false;
		}
		pCmdList->DrawInstanced(3, 1, 0, 0);
	}

	// リソースバリア（レンダーターゲット → プレゼント）
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	pCmdList->ResourceBarrier(1, &barrier);

	// コマンドリストのクローズ
	pCmdList->Close();

	// コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { pCmdList };
	pCmdQueue->ExecuteCommandLists(1, cmdLists);

	// 画面のフリップ
	pSwapChain->Present(1, 0);

	// フェンスによる同期
	const uint64_t fence = fenceCounter[m_FrameIndex];
	pCmdQueue->Signal(pFence, fence);
	fenceCounter[m_FrameIndex]++;

	if (pFence->GetCompletedValue() < fence) {
		pFence->SetEventOnCompletion(fence, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

bool Application::Release(void)
{
	// フェンスの同期を待つ
	if (pFence && fenceEvent) {
		for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
			const uint64_t fence = fenceCounter[i];
			pCmdQueue->Signal(pFence, fence);
			if (pFence->GetCompletedValue() < fence) {
				pFence->SetEventOnCompletion(fence, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
			}
		}
	}

	// リソースの解放
	if (pPSO) {
		pPSO->Release();
		pPSO = nullptr;
	}
	if (pRootSignature) {
		pRootSignature->Release();
		pRootSignature = nullptr;
	}
	if (pVB) {
		pVB->Release();
		pVB = nullptr;
	}
	for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
		if (pCB[i]) {
			pCB[i]->Release();
			pCB[i] = nullptr;
		}
	}
	if (pHeapCBV) {
		pHeapCBV->Release();
		pHeapCBV = nullptr;
	}
	if (pRtvHeap) {
		pRtvHeap->Release();
		pRtvHeap = nullptr;
	}
	if (pCmdList) {
		pCmdList->Release();
		pCmdList = nullptr;
	}
	for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
		if (pCmdAllocator[i]) {
			pCmdAllocator[i]->Release();
			pCmdAllocator[i] = nullptr;
		}
	}
	for (int i = 0; i < FRAME_BUFF_COUNT; ++i) {
		if (pColorBuffer[i]) {
			pColorBuffer[i]->Release();
			pColorBuffer[i] = nullptr;
		}
	}
	if (pSwapChain) {
		pSwapChain->Release();
		pSwapChain = nullptr;
	}
	if (pCmdQueue) {
		pCmdQueue->Release();
		pCmdQueue = nullptr;
	}
	if (pFactory) {
		pFactory->Release();
		pFactory = nullptr;
	}
	if (pDevice) {
		pDevice->Release();
		pDevice = nullptr;
	}
	if (pFence) {
		pFence->Release();
		pFence = nullptr;
	}
	if (fenceEvent) {
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}

	return true;
}