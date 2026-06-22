#include <tchar.h>
#include <cmath>
#include "DxLib.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"
#include "CollisionWorld.h"
#include "PhysicsManager.h"
#include "FogManager.h"

// ===== 定数 =====
const int SCREEN_W = 1280;
const int SCREEN_H = 720;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// ===== 初期化 =====
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(TRUE);
	SetGraphMode(SCREEN_W, SCREEN_H, 32);
	SetUseDirect3DVersion(DX_DIRECT3D_11);

	if (DxLib_Init() == -1) return -1;

	// Zバッファ有効
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetMouseDispFlag(FALSE); // マウス非表示
	SetDrawScreen(DX_SCREEN_BACK);

	// ===== オブジェクト生成 =====
	Player player;
	Camera camera;
	Stage stage;
	FogManager fog;	// フォグ（霧）設定

	stage.Init(&player);
	player.Init(&stage.GetCollisionWorld(), &stage, &camera);
	camera.Init();
	fog.Init();

	// ===== deltaTime用 =====
	LONGLONG prevTime = GetNowHiPerformanceCount();

	// ===== メインループ =====
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// --- 開始時間 ---
		LONGLONG nowTime = GetNowHiPerformanceCount();

		float deltaTime = (float)(nowTime - prevTime) * 0.000001f;

		prevTime = nowTime;

		// --- 更新 ---
		player.Update(deltaTime, camera.GetYaw(), stage.GetPhysics());
		stage.Update(deltaTime, player.GetPos());
		camera.Update(deltaTime, player.GetPos());
		fog.Update(camera.GetPosition(), player.GetPos());

		// --- 描画 ---
		ClearDrawScreen();

		stage.Draw(camera.GetPosition());
		player.Draw();

		stage.DebugDraw();

		player.DebugDraw();
		player.DrawCapsuleDebug(stage.GetEnemies());

		// デバッグ表示
		VECTOR camPos = camera.GetPosition();
		DrawFormatString(20, 360, GetColor(255, 255, 255), "CamPos: X=%.2f Y=%.2f Z=%.2f", camPos.x, camPos.y, camPos.z);
		DrawFormatString(20, 340, GetColor(255, 255, 255), "deltaTime : %.4f", deltaTime);

		ScreenFlip();

		// 60FPS待機
		while (GetNowHiPerformanceCount() - nowTime < 16667)
		{

		}
	}

	DxLib_End();
	return 0;
}
