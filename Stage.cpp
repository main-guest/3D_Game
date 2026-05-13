#include "Stage.h"
#include "Player.h"

void Stage::Init(Player* p)
{
	// ===== ínå` =====
	collisionWorld.Init();

	// ===== ï®óù =====
	physics.Init(&collisionWorld);

	player = p;

	// ===== Enemyê∂ê¨ =====
	auto enemy1 = std::make_unique<Enemy>();
	enemy1->Init(VGet(1000, 0, 1000));
	enemies.push_back(std::move(enemy1));

	auto enemy2 = std::make_unique<Enemy>();
	enemy2->Init(VGet(-1000, 0, -1000));
	enemies.push_back(std::move(enemy2));
}

void Stage::Update(float dt, VECTOR playerPos)
{
	// ===== EnemyçXêV =====
	for (auto& enemy : enemies)
	{
		enemy->Update(dt, playerPos, physics);
	}

	// ===== PlayerçUåÇîªíË =====
	if (player)
	{
		player->CheckAttackHit(enemies);
	}
}

void Stage::Draw()
{
	// ===== Objectï`âÊ =====
	collisionWorld.Draw();

	// ===== Enemyï`âÊ =====
	for (auto& enemy : enemies)
	{
		enemy->Draw();
	}
}