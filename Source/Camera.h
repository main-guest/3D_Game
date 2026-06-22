#pragma once
#include "Dxlib.h"

class Enemy;

class Camera
{
public:
	void Init();

	void Update(float dt, VECTOR playerPos);

	float GetYaw() const { return yaw; }

	int GetMouseDeltaX() const { return mouseDeltaX; }
	int GetMouseDeltaY() const { return mouseDeltaY; }

	VECTOR GetPosition() const { return cameraPos; }

	void SetLockTarget(Enemy* enemy);

	void ClearLockTarget();

private:

	float yaw = 0.0f;
	float pitch = 20.0f * DX_PI / 180.0f;

	float distance = 270.0f;

	VECTOR cameraPos = VGet(0, 0, 0);
	VECTOR cameraTarget = VGet(0, 0, 0);

	bool sideCamera = false;
	int oldF1 = 0;

	// ===== ロックオン =====
	bool isLockOn = false;
	Enemy* lockOnTarget = nullptr;

	// ===== マウス移動量 =====
	int mouseDeltaX = 0;
	int mouseDeltaY = 0;

	// ===== カメラ高さ =====
	float currentCameraHeight = 140.0f;   // 現在の高さ
	float targetCameraHeight = 140.0f;    // 目標高さ

	// ===== 横オフセット =====
	float currentSideOffset = 0.0f;      // 現在の横オフセット
	float targetSideOffset = 0.0f;      // 現在の横オフセット

	// ===== カメラ距離 =====
	float currentDistance = 270.0f;
	float targetDistance = 270.0f;

	// 補間速度
	float cameraOffsetSpeed = 6.0f;
	float sideOffsetSpeed = 1.2f;
};