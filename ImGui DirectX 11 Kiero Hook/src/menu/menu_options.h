#pragma once

namespace MenuOptions {
	namespace ESP {
		inline bool enabled = true;
		inline bool drawBoxes = true;
		inline bool drawHealthBars = true;
		inline bool drawNames = true;
		inline bool drawDistances = false;
		inline bool drawSkeletons = false;

		// Box esp options
		inline float ennemyBoxColor[3] = { 90.f / 255.f, 0.0f, 1.f };
		inline float teamBoxColor[3] = { 0.0f, 1.0f, 0.0f };
		inline float boxBgColor[3] = { 0.0f, 0.0f, 0.0f };
		
		static const char* boxOptions[] = { "2D", "3D" };

		inline float boxOpacity = 1.f;
		inline float boxBgOpacity = 0.4f;
		inline float boxThickness = 1.0f;
		inline float boxLengthMultiplier = 0.4f;
		inline float boxWidthMultiplier = 0.6f;
		
		inline bool boxEnabled = true;
		inline bool boxSkipTeammates = true;
		inline bool boxBgEnabled = true;

		// Health bar options
		inline float healthBarColor[3] = { 0.0f, 1.0f, 0.0f };
		inline float healthBarLowColor[3] = { 1.0f, 0.0f, 0.0f };

		inline bool healthBarEnabled = true;
	}
}