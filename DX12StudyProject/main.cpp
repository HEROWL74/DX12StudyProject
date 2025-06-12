#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Application app;

    // �E�B���h�E�̍쐬
    if (!app.Create(hInstance, nCmdShow)) {
        return -1;
    }

    // D3D12�V�X�e���̏�����
    if (!app.SystemInit()) {
        return -1;
    }

    // ���C�����[�v
    MSG msg = app.Run();

    return static_cast<int>(msg.wParam);
}