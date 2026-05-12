#include "Camera.h"
#include <cmath>

Camera::Camera()
	:yaw(0.0f),
	pitch(20.0f * DX_PI / 180.0f),
	distance(100.0f),
	cameraPos(VGet(0, 0, 0)),
	cameraTarget(VGet(0, 0, 0)),
	sideCamera(false),
	oldF1(0)
{

}

Camera::~Camera()
{

}

void Camera::Init()
{
	SetCameraNearFar(10.0f, 8000.0f);
}

void Camera::Update(VECTOR playerPos)
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

	int dx = mouseX - centerX;
	int dy = mouseY - centerY;

	float sensi = 0.001f;

	yaw += dx * sensi;
	pitch += dy * sensi;

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
	}
	else
	{
		// ===== 通常カメラ =====
		// forwardベクトル
		VECTOR forward;
		forward.x = cosf(pitch) * sinf(yaw);
		forward.y = sinf(pitch);
		forward.z = cosf(pitch) * cosf(yaw);

		// 注視点（プレイヤーの上）
		cameraTarget = VAdd(playerPos, VGet(0.0f, 170.0f, 0.0f));

		// カメラ位置
		cameraPos = VSub(cameraTarget, VScale(forward, distance));
		cameraPos.y += 80.0f;

		SetCameraPositionAndTarget_UpVecY(cameraPos, cameraTarget);
	}
}