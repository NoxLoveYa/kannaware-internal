#include "menu.h"

namespace Menu
{
	void RenderMenu(bool g_unloading)
	{
		// If unloading, skip menu rendering
		if (g_unloading)
			return;

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
}