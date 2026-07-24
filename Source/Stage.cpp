#include "Stage.h"
#include "Player.h"
#include "Camera.h"

Stage::~Stage()
{
	sky.Release();
}

void Stage::Init(Player* p)
{
	// ===== 地形 =====
	collisionWorld.Init();

	// ===== スカイドーム =====
	sky.Init("Assets/mv1Model/SkyDome.mv1");

	// ===== 物理 =====
	physics.Init(&collisionWorld);

	player = p;

	// ===== 天候設定 =====
	fog.SetWeather(0);

	// ===== Enemy生成 =====
	auto enemy1 = std::make_unique<Enemy>();
	enemy1->Init(VGet(1000, 0, 1000));
	enemies.push_back(std::move(enemy1));

	/*auto enemy2 = std::make_unique<Enemy>();
	enemy2->Init(VGet(-1000, 0, -1000));
	enemies.push_back(std::move(enemy2));*/
}

void Stage::Update(float dt, VECTOR playerPos)
{
	// ===== Enemy更新 =====
	for (auto& enemy : enemies)
	{
		enemy->Update(dt, playerPos, physics, player);
	}

	// ===== キャラ衝突用リスト作成 =====
	std::vector<VECTOR> enemyPositions;

	for (auto& enemy : enemies)
	{
		if (!enemy->IsDead())
		{
			enemyPositions.push_back(enemy->GetPos());
		}
	}

	// ===== Player押し返し =====
	if (player)
	{
		VECTOR p = player->GetPos();
		VECTOR v = player->GetVelocity();

		physics.ResolveCharacterCollision(p, v, player->GetRadius(), player->GetHeight(), enemyPositions, 140.0f );  // Enemy height = 140.0f

		// 位置反映
		player->SetPos(p);
		player->SetVelocity(v);
	}

	// ===== Player攻撃判定 =====
	if (player)
	{
		player->CheckAttackHit(enemies);
	}

	// ===== Enemy攻撃判定 =====
	if (player)
	{
		for (auto& enemy : enemies)
		{
			if (!enemy->IsDead())
			{
				enemy->CheckAttackHit(player);
			}
		}
	}

	for (auto& enemy : enemies)
	{
		if (enemy->IsDead())
		{
			// Playerのロックオン解除
			if (player->GetLockOnTarget() == enemy.get())
			{
				player->SetLockOnTarget(nullptr);
			}

			// Cameraのターゲット解除
			if (camera)
			{
				if (camera->GetTargetEnemy() == enemy.get())
				{
					camera->SetLockTarget(nullptr);
				}
			}
		}
	}

	enemies.erase(
		std::remove_if(
			enemies.begin(),
			enemies.end(),
			[](const std::unique_ptr<Enemy>& e)
			{
				return e->IsDead();
			}),
		enemies.end());
}

void Stage::Draw(VECTOR camPos)
{
	// ===== SkyDome描画 =====
	sky.Draw(camPos);

	// ===== Object描画 =====
	collisionWorld.Draw();

	// ===== Enemy描画 =====
	for (auto& enemy : enemies)
	{
		enemy->Render();
	}
}

std::vector<std::unique_ptr<Enemy>>& Stage::GetEnemies()
{
	return enemies;
}

void Stage::DebugDraw()
{
	collisionWorld.DebugDraw();
}

Enemy* Stage::GetBoss()
{
	if (enemies.empty())
	{
		return nullptr;
	}

	return enemies[0].get();
}