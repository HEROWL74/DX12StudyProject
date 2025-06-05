#pragma once
#include <Windows.h>
#include <WinUser.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")



class Application
{
public:
	static constexpr int WINDOW_WID = 800;
	static constexpr int WINDOW_HIG = 600;

	static constexpr TCHAR tszAplName[] =  "step01" ;

	Application(void);
	~Application(void);

	bool Create(HINSTANCE hInst, int nCmdShow);
	bool SystemInit(void);
	MSG Run(void);
	void Render(void);
	bool Release(void);

private:
	HWND hMainWnd; //メンウィンドウのハンドル

	ID3D12Device* pDevice;//デバイス
	IDXGIFactory6* pFactory; //DXGIファクトリー
	ID3D12CommandQueue* pCmdQueue; //コマンドキュー
	IDXGISwapChain4* pSwapChain;//スワップチェイン
	ID3D12CommandAllocator* pCmdAllocator; //コマンドアロケータ
	ID3D12GraphicsCommandList* pCmdList; //コマンドリスト
	ID3D12DescriptorHeap* pRtvHeap;//ディスクリプタヒープ
	ID3D12Fence* pFence;//フェンス
	HANDLE fenceEvent; //フェンスイベント
};
