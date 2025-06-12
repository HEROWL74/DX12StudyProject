#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Application app;

    // ウィンドウの作成
    if (!app.Create(hInstance, nCmdShow)) {
        return -1;
    }

    // D3D12システムの初期化
    if (!app.SystemInit()) {
        return -1;
    }

    // メインループ
    MSG msg = app.Run();

    return static_cast<int>(msg.wParam);
}