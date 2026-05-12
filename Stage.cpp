#include "Stage.h"

Stage::Stage()
{

}

Stage::~Stage()
{

}

void Stage::Init()
{
	// ===== Object =====
	object.Init();

	// ===== Enemyê∂ê¨ =====
	auto enemy1 = std::make_unique<Enemy>();
	enemy1->Init(VGet(600, 0, 600));
	enemies.push_back(std::move(enemy1));

	auto enemy2 = std::make_unique<Enemy>();
	enemy2->Init(VGet(-600, 0, -600));
	enemies.push_back(std::move(enemy2));
}

void Stage::Update(float deltaTime, VECTOR playerPos)
{
	// ===== EnemyçXêV =====
	for (auto& enemy : enemies)
	{
		enemy->Update(deltaTime, object, playerPos);
	}
}

void Stage::Draw()
{
	// ===== Objectï`âÊ =====
	object.Draw();

	// ===== Enemyï`âÊ =====
	for (auto& enemy : enemies)
	{
		enemy->Draw();
	}
}