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
		ImGui::Begin("kannaware", &menu_opened, ImGuiWindowFlags_NoCollapse || ImGuiWindowFlags_NoTitleBar);

		// Tab bar
		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable)) {
			if (ImGui::BeginTabItem("visuals")) {
				// ESP Section
				ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "esp");
				ImGui::Separator();

				ImGui::Text("box settings:");
				if (ImGui::Checkbox("boxes", &MenuOptions::ESP::boxEnabled) || MenuOptions::ESP::boxEnabled) {
					ImGui::Spacing();

					ImGui::Checkbox("boxes bakground", &MenuOptions::ESP::boxBgEnabled);

					ImGui::Spacing();

					ImGui::Checkbox("skip teammates", &MenuOptions::ESP::boxSkipTeammates);
					
					ImGui::Spacing();

					ImGui::Text("box size settings:");

					ImGui::SliderFloat("box thickness", &MenuOptions::ESP::boxThickness, 1.0f, 5.0f);
				
					ImGui::Spacing();

					ImGui::SliderFloat("box length multiplier", &MenuOptions::ESP::boxLengthMultiplier, 0.f, 1.0f);
					ImGui::SliderFloat("box width multiplier", &MenuOptions::ESP::boxWidthMultiplier, 0.f, 1.0f);

					ImGui::Spacing();

					ImGui::Text("box opacity settings:");
					
					ImGui::SliderFloat("box opacity", &MenuOptions::ESP::boxOpacity, 0.0f, 1.0f);
					ImGui::SliderFloat("box background opacity", &MenuOptions::ESP::boxBgOpacity, 0.0f, 1.0f);

					ImGui::Spacing();

					ImGui::Text("box colors:");
					ImGui::ColorEdit3("enemy box color", MenuOptions::ESP::ennemyBoxColor);
					ImGui::ColorEdit3("team box color", MenuOptions::ESP::teamBoxColor);

				}


				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::Spacing();
		ImGui::Spacing();



		// Flags multi-select dropdown
		/*ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "flags");
		if (ImGui::BeginCombo("##flags_combo", flagOptions[gMenuState.esp_flags])) {
			for (int i = 0; i < 6; i++) {
				if (ImGui::Selectable(flagOptions[i], i == gMenuState.esp_flags)) {
					gMenuState.esp_flags = i;
				}
			}
			ImGui::EndCombo();
		}*/
		ImGui::End();
	}
}