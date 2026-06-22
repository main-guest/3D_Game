#include <cmath>
#include "Camera.h"
#include "Enemy.h"

void Camera::Init()
{
	SetCameraNearFar(1.0f, 50000.0f);
}

void Camera::Update(float dt, VECTOR playerPos)
{
	// ===== F1切替 =====
	int nowF1 = CheckHitKey(KEY_INPUT_F1);
	if (nowF1 == 1 && oldF1 == 0)
	{
		sideCamera = !sideCamera;
	}

	oldF1 = nowF1;

	// ===== マウス入力 =====
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// 画面中心（ウィンドウサイズに合わせて調整）
	int w, h;
	GetDrawScreenSize(&w, &h);
	
	int centerX = w / 2;
	int centerY = h / 2;

	mouseDeltaX = mouseX - centerX;
	mouseDeltaY = mouseY - centerY;

	float sensi = 0.001f;

	// ===== ロックオン =====
	if (isLockOn && lockOnTarget)
	{
		VECTOR playerTarget = VAdd(playerPos, VGet(0, 110, 0));

		VECTOR enemyTarget = lockOnTarget->GetCenterPos();

		enemyTarget.y += 60.0f;

		// PlayerからEnemyへの方向
		VECTOR dir = VSub(enemyTarget, playerTarget);

		// 水平方向距離
		float horizontal = sqrtf(dir.x * dir.x + dir.z * dir.z);

		// 目標角度
		float targetYaw = atan2f(dir.x, dir.z);
		float targetPitch = atan2f(dir.y, horizontal);

		// ----- Yaw補間 -----
		float diff = targetYaw - yaw;

		while (diff > DX_PI)diff -= DX_TWO_PI;
		while (diff < -DX_PI)diff += DX_TWO_PI;

		const float rotateSpeed = 8.0f;

		yaw += diff * rotateSpeed * dt;
		
		pitch += (targetPitch - pitch) * rotateSpeed * dt;
	}
	else
	{
		// 通常カメラ
		yaw += mouseDeltaX * sensi;
		pitch += mouseDeltaY * sensi;
	}

	// ===== カメラ高さ・横オフセット更新 =====
	if (isLockOn && lockOnTarget)
	{
		VECTOR enemyPos = lockOnTarget->GetCenterPos();

		float dist = VSize(VSub(enemyPos, playerPos));

		// ===== 距離に応じて高さ変更 =====
		if (dist < 400.0f)
		{
			float t = (400.0f - dist) / 250.0f;

			if (t > 1.0f)
				t = 1.0f;

			targetCameraHeight = 140.0f + t * 220.0f;

			targetDistance = 270.0f + t * 120.0f;
		}
		else
		{
			targetCameraHeight = 140.0f;
			targetDistance = 270.0f;
		}

		// ===== 左右入力による横オフセット =====
		if (CheckHitKey(KEY_INPUT_A))
		{
			targetSideOffset = 100.0f;
		}
		else if (CheckHitKey(KEY_INPUT_D))
		{
			targetSideOffset = -100.0f;
		}
		else
		{
			targetSideOffset = 0.0f;
		}
	}
	else
	{
		targetCameraHeight = 140.0f;
		targetSideOffset = 0.0f;
		targetDistance = 270.0f;
	}

	currentCameraHeight +=
		(targetCameraHeight - currentCameraHeight) * cameraOffsetSpeed * dt;

	currentSideOffset +=
		(targetSideOffset - currentSideOffset) * sideOffsetSpeed * dt;

	currentDistance +=
		(targetDistance - currentDistance) * cameraOffsetSpeed * dt;

	// 上下制限
	if (pitch > 1.2f)pitch = 1.2f;
	if (pitch < -1.2f)pitch = -1.2f;

	SetMousePoint(centerX, centerY);

	if (sideCamera)
	{
		// ===== 横カメラ =====
		cameraTarget = VAdd(playerPos, VGet(0, 0, 0));

		cameraPos = VAdd(cameraTarget, VGet(-400, 0, 0));

		SetCameraPositionAndTarget_UpVecY(cameraPos, cameraTarget);

		return;
	}
	else
	{
		// ===== 通常カメラ =====
		// forwardベクトル
		VECTOR forward;

		forward.x = cosf(pitch) * sinf(yaw);
		forward.y = sinf(pitch);
		forward.z = cosf(pitch) * cosf(yaw);

		VECTOR playerTarget = VAdd(playerPos, VGet(0.0f, 140.0f, 0.0f));

		VECTOR right;

		right.x = cosf(yaw);
		right.y = 0.0f;
		right.z = -sinf(yaw);

		// 注視点（プレイヤーの上）
		cameraTarget = VAdd(playerTarget, VScale(right, currentSideOffset));

		// カメラ位置
		cameraPos = VSub(cameraTarget, VScale(forward, currentDistance));
		cameraPos.y += currentCameraHeight;

		SetCameraPositionAndTarget_UpVecY(cameraPos, cameraTarget);
	}
}

void Camera::SetLockTarget(Enemy* enemy)
{
	lockOnTarget = enemy;

	isLockOn = (enemy != nullptr);
}

void Camera::ClearLockTarget()
{
	lockOnTarget = nullptr;
	
	isLockOn = false;
}