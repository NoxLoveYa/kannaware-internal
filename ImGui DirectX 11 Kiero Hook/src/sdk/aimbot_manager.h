#pragma once

#include "entity_manager.h"

class AimbotManager {
	private:
		EntityManager* entityManager;
public:
	AimbotManager(EntityManager* em) : entityManager(em) {}

	float GetFOVToTarget(uintptr_t targetPawn) {
		if (!entityManager) return FLT_MAX;
		uintptr_t localPawn = entityManager->GetLocalPlayerPawn();
		if (!localPawn || !targetPawn) return FLT_MAX;
		Vector3 localViewAngles = MathUtils::GetViewAngles(localPawn);
		Vector3 targetHeadPos = entityManager->GetBonePosition(targetPawn, "head");
		Vector3 localEyePos = MathUtils::GetEyePosition(localPawn);
		Vector3 directionToTarget = targetHeadPos - localEyePos;
		directionToTarget.Normalize();
		Vector3 forward;
		MathUtils::AngleVectors(localViewAngles, forward);
		float dotProduct = forward.Dot(directionToTarget);
		float fov = acosf(dotProduct) * (180.0f / 3.14159265f); // Convert to degrees
		return fov;
	}
};