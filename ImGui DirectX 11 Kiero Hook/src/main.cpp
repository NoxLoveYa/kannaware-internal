#include "includes.h"
#include <stdio.h>
#include <atomic>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef BOOL(WINAPI* peekMessageA_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef BOOL(WINAPI* peekMessageW_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef void(__fastcall* FrameStageNotify_t)(void*, int);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
peekMessageA_t oPeekMessageA = nullptr;
peekMessageW_t oPeekMessageW = nullptr;
std::atomic<FrameStageNotify_t> oFrameStageNotify{ nullptr };
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

bool g_frameStageNotifyCalled = false;
int g_frameStageCounter = 0;

HMODULE g_hModule = NULL;
volatile bool g_unloading = false;
volatile LONG g_inPresent = 0;

uintptr_t g_clientBase = 0;

enum ClientFrameStage_t {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END,
	FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE
};

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);

	ImGuiStyle& style = ImGui::GetStyle();
	Menu::applyStyling(style);
}

DWORD WINAPI UnloadThread(LPVOID lpReserved)
{
	if (g_unloading)
		return 0;

	g_unloading = true;
	kiero::shutdown();

	oFrameStageNotify.store(nullptr, std::memory_order_release);

	const int maxWaitIterations = 100;
	int waitIter = 0;
	while (InterlockedCompareExchange(&g_inPresent, 0, 0) != 0 && waitIter < maxWaitIterations)
	{
		Sleep(20);
		waitIter++;
	}

	if (oWndProc && window)
	{
		SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
		oWndProc = nullptr;
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = nullptr; }
	if (pContext) { pContext->Release(); pContext = nullptr; }
	if (pDevice) { pDevice->Release(); pDevice = nullptr; }

	Sleep(50);

	if (g_hModule)
		FreeLibraryAndExitThread(g_hModule, 0);

	return 0;
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL WINAPI hkPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL ret = FALSE;
	if (oPeekMessageA)
		ret = oPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	else
		ret = PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	if (ret && lpMsg)
	{
		if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_INSERT)
		{
			Menu::menu_opened = !Menu::menu_opened;
		}
		else if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_DELETE)
		{
			if (!g_unloading)
				CreateThread(nullptr, 0, UnloadThread, nullptr, 0, nullptr);
		}
		else if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_HOME)
		{
			g_ESP.SetEnabled(!g_ESP.IsEnabled());
		}
	}

	return ret;
}

BOOL WINAPI hkPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL ret = FALSE;
	if (oPeekMessageW)
		ret = oPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	else
		ret = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	if (ret && lpMsg)
	{
		if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_INSERT)
		{
			Menu::menu_opened = !Menu::menu_opened;
		}
		else if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_DELETE)
		{
			if (!g_unloading)
				CreateThread(nullptr, 0, UnloadThread, nullptr, 0, nullptr);
		}
		else if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_HOME)
		{
			g_ESP.SetEnabled(!g_ESP.IsEnabled());
		}
	}

	return ret;
}

// FrameStageNotify hook - Update entities here
void __fastcall hkFrameStageNotify(void* thisptr, int stage)
{
	FrameStageNotify_t orig = oFrameStageNotify.load(std::memory_order_acquire);

	if (g_unloading)
	{
		if (orig) orig(thisptr, stage);
		return;
	}

	// CRITICAL: Skip incrementing counter and just pass through during these stages
	// These stages happen during loading and the job system might be unstable
	if (stage == FRAME_UNDEFINED || stage == FRAME_START)
	{
		if (orig) orig(thisptr, stage);
		return;
	}

	g_frameStageNotifyCalled = true;
	g_frameStageCounter++;


	// Do custom logic AFTER the original function completes
	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		// Update entities after network data is received
		if (g_clientBase) {
			g_EntityManager.UpdateEntities();
		}
		break;
	case FRAME_RENDER_START:
		// Update view matrix before rendering
		if (g_clientBase) {
			g_ESP.UpdateViewMatrix(g_clientBase);
		}
		break;
	case FRAME_RENDER_END:
		break;
	}
	if (orig) orig(thisptr, stage);
}

void RenderESPDebugInfo()
{
	if (g_unloading)
		return;

	ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	char debugText[256];
	sprintf_s(debugText,
		"ESP: %s (HOME to toggle)\n"
		"FrameStageNotify: %s\n"
		"Calls: %d\n"
		"Client Base: 0x%llX",
		g_ESP.IsEnabled() ? "ON" : "OFF",
		g_frameStageNotifyCalled ? "HOOKED" : "NOT HOOKED",
		g_frameStageCounter,
		g_clientBase);

	ImVec2 textPos(10, 10);
	ImVec2 textSize = ImGui::CalcTextSize(debugText);

	// Background
	drawList->AddRectFilled(
		ImVec2(textPos.x - 5, textPos.y - 2),
		ImVec2(textPos.x + textSize.x + 5, textPos.y + textSize.y + 2),
		IM_COL32(0, 0, 0, 150)
	);

	drawList->AddText(textPos, IM_COL32(0, 255, 0, 255), debugText);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (g_unloading)
		return oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;

	InterlockedIncrement(&g_inPresent);
	HRESULT hr = S_OK;

	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;

			// Set screen size for ESP
			g_ESP.SetScreenSize(sd.BufferDesc.Width, sd.BufferDesc.Height);

			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}
		else
		{
			hr = oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
			InterlockedDecrement(&g_inPresent);
			return hr;
		}
	}

	if (g_unloading)
	{
		hr = oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
		InterlockedDecrement(&g_inPresent);
		return hr;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Render ESP overlay
	g_ESP.Render();

	// Render debug info
	RenderESPDebugInfo();

	if (Menu::menu_opened) {
		Menu::RenderMenu(g_unloading);
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	hr = oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;

	InterlockedDecrement(&g_inPresent);
	return hr;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);

			kiero::bindImport("user32.dll", "PeekMessageA", (void**)&oPeekMessageA, (void*)hkPeekMessageA);
			kiero::bindImport("user32.dll", "PeekMessageW", (void**)&oPeekMessageW, (void*)hkPeekMessageW);

			HMODULE clientDll = GetModuleHandleA("client.dll");
			if (clientDll)
			{
				g_clientBase = reinterpret_cast<uintptr_t>(clientDll);

				typedef void* (*CreateInterfaceFn)(const char*, int*);
				CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(clientDll, "CreateInterface");

				if (CreateInterface)
				{
					const char* versions[] = { "Source2Client002" };
					void* clientInterface = nullptr;

					for (const char* version : versions)
					{
						clientInterface = CreateInterface(version, nullptr);
						if (clientInterface)
						{
							void** vtable = *(void***)clientInterface;
							void* frameStageNotifyAddr = vtable[36];

							if (frameStageNotifyAddr)
							{
								// Use kiero::bindFunction to hook the actual function
								FrameStageNotify_t tmpOrig = nullptr;
								if (kiero::bindFunction(frameStageNotifyAddr, (void**)&tmpOrig, (FrameStageNotify_t*)hkFrameStageNotify) == kiero::Status::Success)
								{
									oFrameStageNotify.store(tmpOrig, std::memory_order_release);
								}
							}

							break;
						}
					}
				}
			}

			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);

		// Reset everything in case of reinjection
		g_hModule = hMod;
		g_unloading = false;
		g_inPresent = 0;
		init = false;
		window = NULL;
		oWndProc = nullptr;
		oPeekMessageA = nullptr;
		oPeekMessageW = nullptr;
		oFrameStageNotify.store(nullptr, std::memory_order_release);
		pDevice = NULL;
		pContext = NULL;
		mainRenderTargetView = nullptr;
		g_frameStageNotifyCalled = false;
		g_frameStageCounter = 0;
		g_clientBase = 0;

		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}