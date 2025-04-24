#include <windows.h>
#include <WinUser.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <string>

using namespace std;
static constexpr TCHAR tszAplName[] = "step01";

static constexpr int WINDOW_WID = 800;
static constexpr int WINDOW_HIG = 600;

//ウィンドウプロシージャのコールバック関数のプロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst,_In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
	//ウィンドウの作成と表示
	WNDCLASSEX wndcex;

	ZeroMemory(&wndcex, sizeof(wndcex));
	wndcex.cbSize = sizeof(WNDCLASSEX);
	wndcex.style = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW;
	wndcex.lpfnWndProc = WndProc;
	wndcex.cbClsExtra = 0;
	wndcex.cbWndExtra = 0;
	wndcex.hInstance = hInst;
	wndcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPLICATION));
	wndcex.hCursor = NULL;
	wndcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndcex.lpszMenuName = NULL;
	wndcex.lpszClassName = tszAplName;
	wndcex.hIconSm = LoadIcon(wndcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wndcex)) return false;

	HWND hwnd = CreateWindowEx(0, tszAplName, tszAplName,
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WID, WINDOW_HIG,
		NULL, NULL, hInst, NULL);

	if (hwnd == NULL)return false;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	string str = "HelloWorld";

	switch (msg) {
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 5, 5, str.c_str(), (int)strlen(str.c_str()));
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}