#include "includes.h"
#include <stdio.h>
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef BOOL(WINAPI* peekMessageA_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef BOOL(WINAPI* peekMessageW_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef void(__fastcall* FrameStageNotify_t)(void*, int);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
peekMessageA_t oPeekMessageA = nullptr;
peekMessageW_t oPeekMessageW = nullptr;
FrameStageNotify_t oFrameStageNotify = nullptr;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

bool menu_opened = true;

// Global flags to test if FrameStageNotify is being called
bool g_frameStageNotifyCalled = false;
int g_frameStageCounter = 0;

// CS2 Frame stage enum
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

// Menu state
struct MenuState {
	int activeTab = 0;

	// ESP section
	bool esp_boxes = true;
	bool esp_health_bar = true;
	bool esp_name = true;
	bool esp_weapon = false;
	int esp_flags = 0;

	// Models section
	bool show_player_models = false;
	bool show_weapon_models = false;
	float brightness = 0.5f;
} gMenuState;

const char* flagOptions[] = { "Money, Armor", "Money", "Armor", "Health", "Weapon", "Bomb" };

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);

	// Custom styling
	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io_style = ImGui::GetIO();

	// Dark theme
	ImGui::StyleColorsDark();

	// Customize colors
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.95f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.73f);

	// Style tweaks
	style.WindowRounding = 0.0f;
	style.FrameRounding = 2.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.FramePadding = ImVec2(4.0f, 3.0f);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
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
			menu_opened = !menu_opened;
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
			menu_opened = !menu_opened;
		}
	}

	return ret;
}

// CS2 FrameStageNotify hook - using __fastcall for x64
void __fastcall hkFrameStageNotify(void* thisptr, int stage)
{
	// Set flag to show hook is working
	g_frameStageNotifyCalled = true;
	g_frameStageCounter++;

	// Your code BEFORE the original call (pre-frame logic)
	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		// Skin changers, model changes before rendering
		// Entity data has been updated, good time to modify
		break;

	case FRAME_RENDER_START:
		// Visual modifications right before rendering
		// Chams, glow, etc.
		break;
	}

	// Call original
	oFrameStageNotify(thisptr, stage);

	// Your code AFTER the original call (post-frame logic)
	switch (stage)
	{
	case FRAME_RENDER_END:
		// After rendering completes
		break;
	}
}

// Simple test function to draw a box
void RenderTestBox()
{
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	// Draw a simple test box in the center of the screen
	ImVec2 screenCenter = ImVec2(960, 540); // Adjust for your resolution
	ImVec2 boxSize = ImVec2(100, 150);

	ImVec2 topLeft = ImVec2(
		screenCenter.x - boxSize.x / 2,
		screenCenter.y - boxSize.y / 2
	);
	ImVec2 bottomRight = ImVec2(
		screenCenter.x + boxSize.x / 2,
		screenCenter.y + boxSize.y / 2
	);

	// Draw box outline
	drawList->AddRect(
		topLeft,
		bottomRight,
		IM_COL32(255, 0, 0, 255), // Red color
		0.0f,
		0,
		2.0f
	);

	// Draw text showing if FrameStageNotify is hooked
	char text[128];
	sprintf_s(text, "FrameStageNotify: %s\nCalls: %d",
		g_frameStageNotifyCalled ? "HOOKED" : "NOT HOOKED",
		g_frameStageCounter);

	drawList->AddText(
		ImVec2(topLeft.x, bottomRight.y + 10),
		IM_COL32(255, 255, 0, 255),
		text
	);
}

void RenderMenu()
{
	ImGui::SetNextWindowPos(ImVec2(80, 60), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(700, 650), ImGuiCond_FirstUseEver);
	ImGui::Begin("kannaware", &menu_opened, ImGuiWindowFlags_NoMove);

	// Tab bar
	if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
		if (ImGui::BeginTabItem("legit")) {
			ImGui::Text("Legit content here");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("visuals")) {
			ImGui::Text("Visuals content here");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("misc")) {
			ImGui::Text("Misc content here");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("skins")) {
			ImGui::Text("Skins content here");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// ESP Section
	ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "esp");
	ImGui::Separator();

	ImGui::Checkbox("boxes", &gMenuState.esp_boxes);
	ImGui::Checkbox("health bar", &gMenuState.esp_health_bar);
	ImGui::Checkbox("name", &gMenuState.esp_name);
	ImGui::Checkbox("weapon", &gMenuState.esp_weapon);

	// Flags multi-select dropdown
	ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "flags");
	if (ImGui::BeginCombo("##flags_combo", flagOptions[gMenuState.esp_flags])) {
		for (int i = 0; i < 6; i++) {
			if (ImGui::Selectable(flagOptions[i], i == gMenuState.esp_flags)) {
				gMenuState.esp_flags = i;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// Models Section
	ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "models");
	ImGui::Separator();

	ImGui::Text("player models");
	ImGui::SameLine(300);
	ImGui::Checkbox("##player_models", &gMenuState.show_player_models);

	ImGui::Text("weapon models");
	ImGui::SameLine(300);
	ImGui::Checkbox("##weapon_models", &gMenuState.show_weapon_models);

	ImGui::Spacing();

	ImGui::Text("brightness");
	ImGui::SliderFloat("##brightness_slider", &gMenuState.brightness, 0.0f, 1.0f);

	ImGui::End();
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Render test box to show hook is working
	RenderTestBox();

	if (menu_opened) {
		RenderMenu();
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

// Pattern scanning for CS2
uintptr_t FindPattern(HMODULE module, const char* pattern, const char* mask)
{
	//MODULEINFO modInfo = {};
	//GetModuleInformation(GetCurrentProcess(), module, &modInfo, sizeof(MODULEINFO));

	//uintptr_t base = (uintptr_t)module;
	//uintptr_t size = modInfo.SizeOfImage;
	//size_t patternLength = strlen(mask);

	//for (uintptr_t i = 0; i < size - patternLength; i++)
	//{
	//	bool found = true;
	//	for (size_t j = 0; j < patternLength; j++)
	//	{
	//		if (mask[j] != '?' && pattern[j] != *(char*)(base + i + j))
	//		{
	//			found = false;
	//			break;
	//		}
	//	}
	//	if (found)
	//		return base + i;
	//}
	return 0;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			// Hook Present
			kiero::bind(8, (void**)&oPresent, hkPresent);

			// Hook both PeekMessage functions
			kiero::bindImport("user32.dll", "PeekMessageA", (void**)&oPeekMessageA, (void*)hkPeekMessageA);
			kiero::bindImport("user32.dll", "PeekMessageW", (void**)&oPeekMessageW, (void*)hkPeekMessageW);

			// CS2 uses client.dll
			HMODULE clientDll = GetModuleHandleA("client.dll");
			if (clientDll)
			{
				// Method 1: Try CreateInterface (may not work in CS2)
				typedef void* (*CreateInterfaceFn)(const char*, int*);
				CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(clientDll, "CreateInterface");

				void* frameStageNotifyAddr = nullptr;

				if (CreateInterface)
				{
					// CS2 interface versions (try multiple)
					const char* versions[] = { "Source2Client002", "Source2Client001" };
					void* clientInterface = nullptr;

					for (const char* version : versions)
					{
						clientInterface = CreateInterface(version, nullptr);
						if (clientInterface)
						{
							// CS2 uses different vtable index - typically around 5-6
							void** vtable = *(void***)clientInterface;
							frameStageNotifyAddr = vtable[32]; // Try index 5 first
							break;
						}
					}
				}

				// Method 2: Pattern scanning (more reliable for CS2)
				if (!frameStageNotifyAddr)
				{
					MessageBoxA(NULL, "FrameStageNotify not hooked retard L successfully!", "Success", MB_OK);
					// This is an example pattern - you'll need to find the actual CS2 pattern
					// Use a tool like x64dbg or IDA to find the signature
					const char* pattern = "\x40\x53\x48\x83\xEC\x20\x8B\xDA\x48\x8B\x0D";
					const char* mask = "xxxxxxxxxxx";
					frameStageNotifyAddr = (void*)FindPattern(clientDll, pattern, mask);
				}

				// Hook if found
				if (frameStageNotifyAddr)
				{
					if (kiero::bindFunction(frameStageNotifyAddr, (void**)&oFrameStageNotify, (void*)hkFrameStageNotify) == kiero::Status::Success)
					{
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
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}