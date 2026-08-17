#include <tchar.h>
#include <cmath>
#include "DxLib.h"
#include "EffekseerforDxLib.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"
#include "UI.h"
#include "CollisionWorld.h"
#include "PhysicsManager.h"
#include "EffectManager.h"
#include "FogManager.h"
#include "ScreenSize.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// ===== 初期化 =====
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(TRUE);
	SetGraphMode(SCREEN_W, SCREEN_H, 32);
	SetUseDirect3DVersion(DX_DIRECT3D_11);

	if (DxLib_Init() == -1) return -1;

	if (Effekseer_Init(1500) == -1)
	{
		DxLib_End();
		return -1;
	}

	// フルスクリーンウインドウの切り替えでリソースが消えるのを防ぐ。
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	// Zバッファ有効
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetMouseDispFlag(FALSE); // マウス非表示
	SetDrawScreen(DX_SCREEN_BACK);

	// ===== オブジェクト生成 =====
	Player player;
	Camera camera;
	Stage stage;
	UI ui;
	FogManager fog;	// フォグ（霧）設定

	stage.Init(&player);
	player.Init(&stage.GetCollisionWorld(), &stage, &camera);
	camera.Init();
	ui.Init(&player);
	ui.SetEnemy(stage.GetBoss());
	fog.Init();

	EffectManager::Instance().Init();

	EffectManager::Instance().Load("Slash01", "Assets/Effect/Sword/efk/Slash01.efk", 5.0f);

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
		ui.Update(deltaTime, &player);

		if (player.IsLockOn())
		{
			camera.SetLockTarget(player.GetLockOnTarget());
		}
		else
		{
			camera.ClearLockTarget();
		}

		camera.Update(deltaTime, player.GetPos());

		fog.Update(camera.GetPosition(), player.GetPos());

		// DXライブラリのカメラとEffekseerのカメラを同期する。
		Effekseer_Sync3DSetting();

		// Effekseerにより再生中のエフェクトを更新する。
		EffectManager::Instance().Update();

		// --- 描画 ---
		ClearDrawScreen();

		stage.Draw(camera.GetPosition());
		player.Draw();

		EffectManager::Instance().Draw();

		ui.Draw(&player);

		//stage.DebugDraw();

		//camera.DebugDraw();

		//player.DebugDraw();
		//player.DrawCapsuleDebug(stage.GetEnemies());

		// デバッグ表示
		/*VECTOR camPos = camera.GetPosition();
		DrawFormatString(20, 360, GetColor(255, 255, 255), "CamPos: X=%.2f Y=%.2f Z=%.2f", camPos.x, camPos.y, camPos.z);
		DrawFormatString(20, 340, GetColor(255, 255, 255), "deltaTime : %.4f", deltaTime);*/

		ScreenFlip();

		// 60FPS待機
		while (GetNowHiPerformanceCount() - nowTime < 16667)
		{

		}
	}

	EffectManager::Instance().Release();

	Effkseer_End();

	DxLib_End();
	return 0;
}
