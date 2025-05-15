#include <Windows.h>
#include "Application.h"

int WINAPI WinMain(_In_ HINSTANCE hInst,_In_opt_ HINSTANCE hPrevInst,_In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
	Application apl;
	if (!apl.Create(hInst, nCmdShow))return -1;
	if (!apl.SystemInit())return -1;
	MSG msg = apl.Run();
	return (int)msg.wParam;
}

