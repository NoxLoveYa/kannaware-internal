#pragma once
#include "../../ext/imgui/imgui.h"
#include "menu_options.h"

namespace Menu
{
	// Menu state
	static bool menu_opened = true;

	void applyStyling(ImGuiStyle& style);
	void RenderMenu(bool g_unloading);
}