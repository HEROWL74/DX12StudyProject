#include <string>
#include "Application.h"

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
	return true;
}

MSG Application::Run(void)
{
	//���C���̃��b�Z�[�W���[�v
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg;
}

bool Application::Release(void)
{
	return true;
}