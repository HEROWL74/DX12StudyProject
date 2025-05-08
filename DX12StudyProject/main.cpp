#include <Windows.h>
#include "Application.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
    Application app;

    if (!app.Create(hInstance, nCmdShow)) {
        MessageBox(NULL, _T("�E�B���h�E�̍쐬�Ɏ��s���܂����B"), _T("�G���["), MB_OK | MB_ICONERROR);
        return -1;
    }

    if (!app.SystemInit()) {
        MessageBox(NULL, _T("�V�X�e���̏������Ɏ��s���܂����B"), _T("�G���["), MB_OK | MB_ICONERROR);
        return -1;
    }

    MSG msg = app.Run();

    app.Release();

    return (int)msg.wParam;
}
