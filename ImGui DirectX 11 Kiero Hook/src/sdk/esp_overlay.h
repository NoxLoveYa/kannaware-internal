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

    // Colors
    ImU32 enemyColor = IM_COL32(255, 0, 0, 255);
    ImU32 teamColor = IM_COL32(0, 255, 0, 255);
    ImU32 healthBarGreen = IM_COL32(0, 255, 0, 255);
    ImU32 healthBarYellow = IM_COL32(255, 255, 0, 255);
    ImU32 healthBarRed = IM_COL32(255, 0, 0, 255);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    ImU32 boxBg = IM_COL32(0, 0, 0, 100);

    void DrawBox(ImDrawList* drawList, const Vector2& head, const Vector2& feet, ImU32 color) {
        float height = feet.y - head.y;
        float width = height * 0.65f;

        float left = head.x - width * 0.5f;
        float right = head.x + width * 0.5f;
        float top = head.y;
        float bottom = feet.y;

        // Draw box outline
        drawList->AddRect(
            ImVec2(left, top),
            ImVec2(right, bottom),
            color,
            0.0f,
            0,
            2.0f
        );
    }

    void DrawHealthBar(ImDrawList* drawList, const Vector2& head, const Vector2& feet, int health, int maxHealth) {
        float height = feet.y - head.y;
        float width = height * 0.65f;
        float left = head.x - width * 0.5f;

        float barWidth = 4.0f;
        float barHeight = height;
        float barX = left - barWidth - 2.0f;
        float barY = head.y;

        // Background
        drawList->AddRectFilled(
            ImVec2(barX, barY),
            ImVec2(barX + barWidth, barY + barHeight),
            IM_COL32(0, 0, 0, 150)
        );

        // Health fill
        float healthPercent = static_cast<float>(health) / static_cast<float>(maxHealth);
        healthPercent = MathUtils::Clamp(healthPercent, 0.0f, 1.0f);
        float fillHeight = barHeight * healthPercent;

        ImU32 healthColor;
        if (healthPercent > 0.6f) {
            healthColor = healthBarGreen;
        }
        else if (healthPercent > 0.3f) {
            healthColor = healthBarYellow;
        }
        else {
            healthColor = healthBarRed;
        }

        drawList->AddRectFilled(
            ImVec2(barX, barY + barHeight - fillHeight),
            ImVec2(barX + barWidth, barY + barHeight),
            healthColor
        );
    }

    void DrawPlayerInfo(ImDrawList* drawList, const Vector2& head, const PlayerInfo& player) {
        float height = head.y;
        float x = head.x;

        // Draw name above head
        char nameText[128];
        sprintf_s(nameText, "%s", player.name);

        ImVec2 textSize = ImGui::CalcTextSize(nameText);
        ImVec2 textPos(x - textSize.x * 0.5f, height - textSize.y - 5);

        // Text background
        drawList->AddRectFilled(
            ImVec2(textPos.x - 2, textPos.y - 1),
            ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1),
            boxBg
        );

        drawList->AddText(textPos, textColor, nameText);

        // Draw health text
        char healthText[32];
        sprintf_s(healthText, "%d HP", player.health);

        ImVec2 healthSize = ImGui::CalcTextSize(healthText);
        ImVec2 healthPos(x - healthSize.x * 0.5f, height - textSize.y - healthSize.y - 8);

        drawList->AddRectFilled(
            ImVec2(healthPos.x - 2, healthPos.y - 1),
            ImVec2(healthPos.x + healthSize.x + 2, healthPos.y + healthSize.y + 1),
            boxBg
        );

        drawList->AddText(healthPos, textColor, healthText);
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
        int localTeam = -1;
        uintptr_t localPawn = g_EntityManager.GetLocalPlayerPawn();
        if (localPawn) {
            localTeam = *reinterpret_cast<int*>(localPawn + 0x3EB);
        }
        else
            return;

        g_EntityManager.ForEachPlayer([&](const PlayerInfo& player) {
            if (!player.isValid || !player.isAlive) return;

            // Calculate head position (add eye height)
            Vector3 headPos = player.position;
            headPos.z += 75.0f; // Approximate head height

            Vector3 feetPos = player.position;

            // Convert to screen space
            Vector2 headScreen, feetScreen;
            if (!MathUtils::WorldToScreen(headPos, headScreen, viewMatrix, screenWidth, screenHeight))
                return;
            if (!MathUtils::WorldToScreen(feetPos, feetScreen, viewMatrix, screenWidth, screenHeight))
                return;

            // Determine color based on team
            ImU32 boxColor = (player.teamNum == localTeam) ? teamColor : enemyColor;

            // Draw box
            DrawBox(drawList, headScreen, feetScreen, boxColor);

            // Draw health bar
            DrawHealthBar(drawList, headScreen, feetScreen, player.health, player.maxHealth);

            // Draw player info
            DrawPlayerInfo(drawList, headScreen, player);
            });
    }
};

// Global instance
static ESPOverlay g_ESP;