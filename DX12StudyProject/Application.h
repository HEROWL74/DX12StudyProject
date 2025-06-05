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
	HWND hMainWnd; //�����E�B���h�E�̃n���h��

	ID3D12Device* pDevice;//�f�o�C�X
	IDXGIFactory6* pFactory; //DXGI�t�@�N�g���[
	ID3D12CommandQueue* pCmdQueue; //�R�}���h�L���[
	IDXGISwapChain4* pSwapChain;//�X���b�v�`�F�C��
	ID3D12CommandAllocator* pCmdAllocator; //�R�}���h�A���P�[�^
	ID3D12GraphicsCommandList* pCmdList; //�R�}���h���X�g
	ID3D12DescriptorHeap* pRtvHeap;//�f�B�X�N���v�^�q�[�v
	ID3D12Fence* pFence;//�t�F���X
	HANDLE fenceEvent; //�t�F���X�C�x���g
};
