#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include "MinHook.h" // use w minhook    https://github.com/TsudaKageyu/minhook

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

typedef HRESULT(__stdcall *PresentFn)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
PresentFn oPresent = nullptr;

ID3D11Device *pDevice = nullptr;
ID3D11DeviceContext *pContext = nullptr;
ID3D11RenderTargetView *pRenderTargetView = nullptr;

HRESULT __stdcall hkPresent(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!pDevice)
    {
      
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice);
        pDevice->GetImmediateContext(&pContext);

        ID3D11Texture2D *pBackBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);

        if (pBackBuffer)
        {
            pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
            pBackBuffer->Release();
        }
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD_PTR *GetVTable(void *instance, size_t offset = 0)
{
    return *reinterpret_cast<DWORD_PTR **>((DWORD_PTR)instance + offset);
}

void Hook()
{
    //  DX11 
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc,
        0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        L"DummyWindow", NULL};
    RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(wc.lpszClassName, L"Dummy", WS_OVERLAPPEDWINDOW,
                             0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ID3D11Device *pDummyDevice = nullptr;
    ID3D11DeviceContext *pDummyContext = nullptr;
    IDXGISwapChain *pDummySwapChain = nullptr;

    if (D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
            D3D11_SDK_VERSION, &sd, &pDummySwapChain, &pDummyDevice, NULL, &pDummyContext) != S_OK)
    {
        MessageBox(0, L"Failed to create dummy device", 0, 0);
        return;
    }

    DWORD_PTR *pVTable = GetVTable(pDummySwapChain);
    void *pPresentAddr = (void *)pVTable[8]; // 8. index = Present

    MH_Initialize();
    MH_CreateHook(pPresentAddr, &hkPresent, reinterpret_cast<void **>(&oPresent));
    MH_EnableHook(pPresentAddr);

    pDummySwapChain->Release();
    pDummyDevice->Release();
    pDummyContext->Release();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

DWORD WINAPI InitThread(LPVOID)
{
    Hook();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason, LPVOID)
{
    if (ul_reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
    }
    return TRUE;
}


