#pragma once
#include "entity_manager.h"
#include "math_utils.h"
#include "../../ext/imgui/imgui.h"

class ESPOverlay {
private:
    ViewMatrix_t viewMatrix;
    int screenWidth;
    int screenHeight;
    bool enabled = true;

    ImU32 textColor = IM_COL32(255, 255, 255, 255);

    void DrawBox(ImDrawList* drawList, const Vector2& head, const Vector2& feet, bool isEnemy) {
        if (!isEnemy && MenuOptions::ESP::boxSkipTeammates) return;

        float height = feet.y - head.y;
        float width = height * 0.65f;
        float left = head.x - width * 0.5f;
        float right = head.x + width * 0.5f;
        float top = head.y;
        float bottom = feet.y;

        // Get colors
        float* boxColorArray = isEnemy ? MenuOptions::ESP::ennemyBoxColor : MenuOptions::ESP::teamBoxColor;
        ImU32 boxColor = IM_COL32(
            (int)(boxColorArray[0] * 255),
            (int)(boxColorArray[1] * 255),
            (int)(boxColorArray[2] * 255),
            (int)(MenuOptions::ESP::boxOpacity * 255)
        );

        ImU32 bgColor = IM_COL32(
            (int)(MenuOptions::ESP::boxBgColor[0] * 255),
            (int)(MenuOptions::ESP::boxBgColor[1] * 255),
            (int)(MenuOptions::ESP::boxBgColor[2] * 255),
            (int)(MenuOptions::ESP::boxBgOpacity * 255 * MenuOptions::ESP::boxOpacity)
        );

        ImU32 outlineColor = IM_COL32(0, 0, 0, MenuOptions::ESP::boxOpacity * 255);

        // Calculate line lengths based on multipliers
        float horizontalLength = width * MenuOptions::ESP::boxWidthMultiplier * 0.5f;
        float verticalLength = height * MenuOptions::ESP::boxLengthMultiplier * 0.5f;

        // 1. Draw filled background (full box)
        if (MenuOptions::ESP::boxBgEnabled) {
            drawList->AddRectFilled(
                ImVec2(left, top),
                ImVec2(right, bottom),
                bgColor
            );
        }

        float thickness = MenuOptions::ESP::boxThickness;
        float outlineThickness = 1.0f;
        float halfThickness = thickness * 0.5f;

        // Draw all outlines first (outer and inner)
        float halfOutline = outlineThickness * 0.5f;

        // Top-left corner outlines
        // Outer
        drawList->AddLine(ImVec2(left - halfThickness - 1, top - halfThickness - 1), ImVec2(left + horizontalLength + halfThickness + 1 + halfOutline, top - halfThickness - 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(left - halfThickness - 1, top - halfThickness - 1), ImVec2(left - halfThickness - 1, top + verticalLength + halfThickness + 1 + halfOutline), outlineColor, outlineThickness);
        // Inner
        drawList->AddLine(ImVec2(left - halfThickness + 1, top + halfThickness + 1), ImVec2(left + horizontalLength + halfThickness + 1 + halfOutline, top + halfThickness + 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(left + halfThickness + 1, top - halfThickness + 1), ImVec2(left + halfThickness + 1, top + verticalLength + halfThickness + 1 + halfOutline), outlineColor, outlineThickness);

        // Top-right corner outlines
        // Outer
        drawList->AddLine(ImVec2(right - horizontalLength - halfThickness - 1 - halfOutline, top - halfThickness - 1), ImVec2(right + halfThickness + 1, top - halfThickness - 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(right + halfThickness + 1, top - halfThickness - 1), ImVec2(right + halfThickness + 1, top + verticalLength + halfThickness + 1 + halfOutline), outlineColor, outlineThickness);
        // Inner
        drawList->AddLine(ImVec2(right - horizontalLength - halfThickness - 1 - halfOutline, top + halfThickness + 1), ImVec2(right + halfThickness - 1, top + halfThickness + 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(right - halfThickness - 1, top + halfThickness + 1), ImVec2(right - halfThickness - 1, top + verticalLength + halfThickness + 1 + halfOutline), outlineColor, outlineThickness);

        // Bottom-left corner outlines
        // Outer
        drawList->AddLine(ImVec2(left - halfThickness - 1, bottom + halfThickness + 1), ImVec2(left + horizontalLength + halfThickness + 1 + halfOutline, bottom + halfThickness + 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(left - halfThickness - 1, bottom - verticalLength - halfThickness - 1 - halfOutline), ImVec2(left - halfThickness - 1, bottom + halfThickness + 1), outlineColor, outlineThickness);
        // Inner
        drawList->AddLine(ImVec2(left - halfThickness + 1, bottom - halfThickness - 1), ImVec2(left + horizontalLength + halfThickness + 1 + halfOutline, bottom - halfThickness - 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(left + halfThickness + 1, bottom - verticalLength - halfThickness - 1 - halfOutline), ImVec2(left + halfThickness + 1, bottom + halfThickness - 1), outlineColor, outlineThickness);

        // Bottom-right corner outlines
        // Outer
        drawList->AddLine(ImVec2(right - horizontalLength - halfThickness - 1 - halfOutline, bottom + halfThickness + 1), ImVec2(right + halfThickness + 1, bottom + halfThickness + 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(right + halfThickness + 1, bottom - verticalLength - halfThickness - 1 - halfOutline), ImVec2(right + halfThickness + 1, bottom + halfThickness + 1), outlineColor, outlineThickness);
        // Inner
        drawList->AddLine(ImVec2(right - horizontalLength - halfThickness - 1 - halfOutline, bottom - halfThickness - 1), ImVec2(right + halfThickness - 1, bottom - halfThickness - 1), outlineColor, outlineThickness);
        drawList->AddLine(ImVec2(right - halfThickness - 1, bottom - verticalLength - halfThickness - 1 - halfOutline), ImVec2(right - halfThickness - 1, bottom - halfThickness - 1), outlineColor, outlineThickness);

        // Draw all colored lines last (on top) - EXTENDED TO OVERLAP AT CORNERS

        // Top-left corner colored lines (extend horizontal and vertical to overlap)
        drawList->AddLine(ImVec2(left - halfThickness, top), ImVec2(left + horizontalLength + halfThickness, top), boxColor, thickness);
        drawList->AddLine(ImVec2(left, top - halfThickness), ImVec2(left, top + verticalLength + halfThickness), boxColor, thickness);

        // Top-right corner colored lines
        drawList->AddLine(ImVec2(right - horizontalLength - halfThickness, top), ImVec2(right + halfThickness, top), boxColor, thickness);
        drawList->AddLine(ImVec2(right, top - halfThickness), ImVec2(right, top + verticalLength + halfThickness), boxColor, thickness);

        // Bottom-left corner colored lines
        drawList->AddLine(ImVec2(left - halfThickness, bottom), ImVec2(left + horizontalLength + halfThickness, bottom), boxColor, thickness);
        drawList->AddLine(ImVec2(left, bottom - verticalLength - halfThickness), ImVec2(left, bottom + halfThickness), boxColor, thickness);

        // Bottom-right corner colored lines
        drawList->AddLine(ImVec2(right - horizontalLength - halfThickness, bottom), ImVec2(right + halfThickness, bottom), boxColor, thickness);
        drawList->AddLine(ImVec2(right, bottom - verticalLength - halfThickness), ImVec2(right, bottom + halfThickness), boxColor, thickness);
    }

    void DrawHealthBar(ImDrawList* drawList, const Vector2& head, const Vector2& feet, int health, int maxHealth, bool isEnemy) {
        if (!isEnemy && MenuOptions::ESP::boxSkipTeammates) return;
        float height = feet.y - head.y;
        float width = height * 0.65f;
        float left = head.x - width * 0.5f;

        float barWidth = 2.0f;
        float barHeight = height;
        float barX = left - barWidth - 4.0f - MenuOptions::ESP::boxThickness / 2;
        float barY = head.y;
        
        ImU32 healthColor = IM_COL32(
            (int)(MenuOptions::ESP::healthBarColor[0] * 255),
            (int)(MenuOptions::ESP::healthBarColor[1] * 255),
            (int)(MenuOptions::ESP::healthBarColor[2] * 255),
            (int)(255)
        );

        ImU32 healthLowColor = IM_COL32(
            (int)(MenuOptions::ESP::healthBarLowColor[0] * 255),
            (int)(MenuOptions::ESP::healthBarLowColor[1] * 255),
            (int)(MenuOptions::ESP::healthBarLowColor[2] * 255),
            (int)(255)
        );

        ImU32 bgColor = IM_COL32(
            (int)(MenuOptions::ESP::healthBarBgColor[0] * 255),
            (int)(MenuOptions::ESP::healthBarBgColor[1] * 255),
            (int)(MenuOptions::ESP::healthBarBgColor[2] * 255),
            (int)(255)
        );

        // Health fill
        float healthPercent = static_cast<float>(health) / static_cast<float>(maxHealth);
        healthPercent = MathUtils::Clamp(healthPercent, 0.0f, 1.0f);
        float fillHeight = barHeight * healthPercent;

        // Background
        drawList->AddRectFilled(
            ImVec2(barX, barY),
            ImVec2(barX + barWidth, barY + barHeight),
            bgColor
        );

        //outlines
        drawList->AddRectFilled(
            ImVec2(barX - 1.f, barY + barHeight - fillHeight - 1.f),
            ImVec2(barX + barWidth + 2.f, barY + barHeight + 1.f),
            bgColor
        );

		// Health fill
        drawList->AddRectFilled(
            ImVec2(barX, barY + barHeight - fillHeight),
            ImVec2(barX + barWidth, barY + barHeight),
            healthPercent < 0.35f ? healthLowColor : healthColor
		);
    }

    void DrawPlayerInfo(ImDrawList* drawList, const Vector2& head, const Vector2& feetScreen, const PlayerInfo& player, bool isEnemy) {
        if (!isEnemy && MenuOptions::ESP::boxSkipTeammates) return;
        float height = head.y;
        float x = head.x;

        ImU32 bgColor = IM_COL32(
            (int)(MenuOptions::ESP::boxBgColor[0] * 255),
            (int)(MenuOptions::ESP::boxBgColor[1] * 255),
            (int)(MenuOptions::ESP::boxBgColor[2] * 255),
            (int)(MenuOptions::ESP::boxBgOpacity * 255 * MenuOptions::ESP::textBgOpacity)
        );

        ImU32 circleColor = IM_COL32(
            (int)(MenuOptions::ESP::circleColor[0] * 255),
            (int)(MenuOptions::ESP::circleColor[1] * 255),
            (int)(MenuOptions::ESP::circleColor[2] * 255),
            (int)(MenuOptions::ESP::circleOpacity * 255 * MenuOptions::ESP::textBgOpacity)
        );

        if (MenuOptions::ESP::textNameEnabled) {
            // Draw name above head
            char nameText[128];
            sprintf_s(nameText, "%s", player.name);

            ImVec2 textSize = ImGui::CalcTextSize(nameText);
            ImVec2 textPos(x - textSize.x * 0.5f, height - textSize.y - 5);

            // Text background
            drawList->AddRectFilled(
                ImVec2(textPos.x - 2, textPos.y - 1),
                ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1),
                bgColor
            );

            drawList->AddText(textPos, textColor, nameText);
        }

              //if (MenuOptions::ESP::distanceEnabled) {
              //    // Draw distance below feet
              //    float distance = MathUtils::GetDistance(g_EntityManager.GetLocalPlayerPawn(), player.pawn);
              //    char distanceText[32];
              //    sprintf_s(distanceText, "%.1f m", distance);
              //    ImVec2 distanceSize = ImGui::CalcTextSize(distanceText);
              //    ImVec2 distancePos(x - distanceSize.x * 0.5f, feetScreen.y + 2.5f);
              //    drawList->AddRectFilled(
              //        ImVec2(distancePos.x - 2, distancePos.y - 1),
              //        ImVec2(distancePos.x + distanceSize.x + 2, distancePos.y + distanceSize.y + 1),
              //        bgColor
              //    );
              //    drawList->AddText(distancePos, textColor, distanceText);
              //}

        if (MenuOptions::ESP::textHealthEnabled) {
            // Draw health text
            char healthText[32];
            sprintf_s(healthText, "%d HP", player.health);

            ImVec2 healthSize = ImGui::CalcTextSize(healthText);
            ImVec2 healthPos(x - healthSize.x * 0.5f, feetScreen.y + healthSize.y * 0.5f - 2.5f);

            drawList->AddRectFilled(
                ImVec2(healthPos.x - 2, healthPos.y - 1),
                ImVec2(healthPos.x + healthSize.x + 2, healthPos.y + healthSize.y + 1),
                bgColor
            );

            drawList->AddText(healthPos, textColor, healthText);
        }

        if (MenuOptions::ESP::circleEnabled) {
		    Vector3 headPos = player.bonePositions.at("head");
		    Vector2 headScreen;

            if (!MathUtils::WorldToScreen(headPos, headScreen, viewMatrix, screenWidth, screenHeight))
                return;

            ImU32 circleColor = IM_COL32(
                (int)(MenuOptions::ESP::circleColor[0] * 255),
                (int)(MenuOptions::ESP::circleColor[1] * 255),
                (int)(MenuOptions::ESP::circleColor[2] * 255),
                (int)(MenuOptions::ESP::circleOpacity * 255)
            );

		    drawList->AddCircleFilled(ImVec2(headScreen.x, headScreen.y), MenuOptions::ESP::circleRadius, circleColor, 12);
		}
    }

public:
    ESPOverlay() : screenWidth(1920), screenHeight(1080), enabled(true) {
        memset(&viewMatrix, 0, sizeof(viewMatrix));
    }

    void UpdateViewMatrix(uintptr_t clientBase) {
        if (!clientBase) return;

        uintptr_t matrixAddr = clientBase + Offsets::Client::dwViewMatrix;
        viewMatrix = *reinterpret_cast<ViewMatrix_t*>(matrixAddr);
    }

    void SetScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }

    void SetEnabled(bool state) {
        enabled = state;
    }

    bool IsEnabled() const {
        return enabled;
    }

    void Render() {
        if (!enabled) return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        // Get local player team for color coding
		LocalPlayerInfo& localPlayerInfo = g_EntityManager.GetLocalPlayerInfo();
		int localTeam = localPlayerInfo.teamNum;

        if (!localPlayerInfo.isValid || !localPlayerInfo.isAlive)
			return;

        g_EntityManager.ForEachPlayer([&](const PlayerInfo& player) {
            if (!player.isValid || !player.isAlive) return;

            // Calculate head position (add eye height)
            Vector3 headPos = player.position;
            headPos.z += 75.0f; // Approximate head height

            Vector3 feetPos = player.position;
			feetPos.z -= 10.0f; // Feet position

            // Convert to screen space
            Vector2 headScreen, feetScreen;
            if (!MathUtils::WorldToScreen(headPos, headScreen, viewMatrix, screenWidth, screenHeight))
                return;
            if (!MathUtils::WorldToScreen(feetPos, feetScreen, viewMatrix, screenWidth, screenHeight))
                return;

			bool isEnemy = (player.teamNum != localTeam);
            
            if (MenuOptions::ESP::boxEnabled)
                DrawBox(drawList, headScreen, feetScreen, isEnemy);

            // Draw health bar
			if (MenuOptions::ESP::healthBarEnabled)
                DrawHealthBar(drawList, headScreen, feetScreen, player.health, player.maxHealth, isEnemy);

            // Draw player info
            if (MenuOptions::ESP::nameEnabled)
            DrawPlayerInfo(drawList, headScreen, feetScreen, player, isEnemy);
        });
    }
};

// Global instance
static ESPOverlay g_ESP;