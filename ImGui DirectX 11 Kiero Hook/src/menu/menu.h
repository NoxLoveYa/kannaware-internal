#pragma once
#include "../../ext/imgui/imgui.h"

namespace Menu
{
	// Menu state
	static struct MenuState {
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

	static const char* flagOptions[] = { "Money, Armor", "Money", "Armor", "Health", "Weapon", "Bomb" };

	static bool menu_opened = true;

	void applyStyling(ImGuiStyle& style);
	void RenderMenu(bool g_unloading);
}