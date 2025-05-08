#include <Windows.h>
#include "Application.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
    Application app;

    if (!app.Create(hInstance, nCmdShow)) {
        MessageBox(NULL, _T("ウィンドウの作成に失敗しました。"), _T("エラー"), MB_OK | MB_ICONERROR);
        return -1;
    }

    if (!app.SystemInit()) {
        MessageBox(NULL, _T("システムの初期化に失敗しました。"), _T("エラー"), MB_OK | MB_ICONERROR);
        return -1;
    }

    MSG msg = app.Run();

    app.Release();

    return (int)msg.wParam;
}
