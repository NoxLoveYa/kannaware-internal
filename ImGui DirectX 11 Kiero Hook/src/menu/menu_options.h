#pragma once

namespace MenuOptions {
	namespace ESP {
		// Box esp options
		inline float ennemyBoxColor[3] = { 90.f / 255.f, 0.0f, 1.f };
		inline float teamBoxColor[3] = { 0.0f, 1.0f, 0.0f };
		inline float boxBgColor[3] = { 0.0f, 0.0f, 0.0f };

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
		inline float healthBarBgColor[3] = { 0.0f, 0.0f, 0.0f };

		inline bool healthBarEnabled = true;
		inline bool healthBarSkipTeammates = true;
		inline bool healthBarGradientEnabled = true;
		inline bool healthBgEnabled = true;

		// Text esp options
		inline bool nameEnabled = true;

		inline float textBgOpacity = 0.15f;

		inline bool textNameEnabled = true;
		inline bool textHealthEnabled = true;

		// Circle esp options
		inline bool circleEnabled = true;

		inline float circleOpacity = 1.0f;
		inline float circleRadius = 2.0f;

		inline float circleColor[3] = { 1.0f, 1.0f, 1.0f };
	}
}