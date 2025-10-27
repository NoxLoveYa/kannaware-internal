#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "../ext/kiero/kiero.h"
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx11.h"

#include "menu/menu.h"

#include "sdk/offsets.h"
#include "sdk/entity.h"
#include "sdk/math_utils.h"
#include "sdk/entity_manager.h"
#include "sdk/esp_overlay.h"
#include "sdk/vmt_hook.h"
#include "sdk/aimbot_manager.h"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL(WINAPI* peekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef uintptr_t PTR;