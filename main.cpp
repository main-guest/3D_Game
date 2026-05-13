#include <tchar.h>
#include <cmath>
#include "DxLib.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"
#include "CollisionWorld.h"
#include "PhysicsManager.h"

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

	SetMouseDispFlag(FALSE); // マウス非表示
	SetDrawScreen(DX_SCREEN_BACK);

	// ===== 地形生成 =====
	CollisionWorld world;
	world.Init();

	PhysicsManager physics;
	physics.Init(&world);

	// ===== オブジェクト生成 =====
	Player player;
	Camera camera;
	Stage stage;

	player.Init(&world);
	camera.Init();

	stage.Init(world, physics);

	// ===== deltaTime用 =====
	LONGLONG prevTime = GetNowHiPerformanceCount();

	// ===== メインループ =====
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// --- 開始時間 ---
		LONGLONG nowTime = GetNowHiPerformanceCount();

		float deltaTime = (nowTime - prevTime) / 1000000.0f;

		prevTime = nowTime;

		// --- 更新 ---
		camera.Update(player.GetPos());
		player.Update(deltaTime, camera.GetYaw(), physics);
		stage.Update(deltaTime, player.GetPos());

		// --- 描画 ---
		ClearDrawScreen();

		stage.Draw();
		player.Draw();

		// デバッグ表示
		VECTOR camPos = camera.GetPosition();
		DrawFormatString(0, 360, GetColor(255, 255, 255), _T("CamPos: X=%.2f Y=%.2f Z=%.2f"), camPos.x, camPos.y, camPos.z);
		DrawFormatString(0, 340, GetColor(255, 255, 255), _T("deltaTime : %.4f"), deltaTime);

		ScreenFlip();

		// 60FPS待機
		while (GetNowHiPerformanceCount() - nowTime < 16667)
		{

		}
	}

	DxLib_End();
	return 0;
}
