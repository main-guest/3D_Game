#pragma once
#include <vector>
#include <memory>
#include "CollisionWorld.h"
#include "PhysicsManager.h"
#include "Enemy.h"
#include "SkyDome.h"
#include "FogManager.h"

class Player;

class Stage
{
public:
	~Stage();

	void Init(Player* p);

	void Update(float dt, VECTOR playerPos);

	void Draw(VECTOR camPos);

	CollisionWorld& GetCollisionWorld()
	{
		return collisionWorld;
	}

	PhysicsManager& GetPhysics()
	{
		return physics;
	}

	void DebugDraw();

	std::vector<std::unique_ptr<Enemy>>& GetEnemies();

private:
	// ===== ステージオブジェクト =====
	CollisionWorld collisionWorld;
	PhysicsManager physics;

	// ===== スカイドーム =====
	SkyDome sky;

	// ===== フォグ =====
	FogManager fog;

	// ===== Player参照 =====
	Player* player = nullptr;

	// ===== Enemy一覧 =====
	std::vector<std::unique_ptr<Enemy>> enemies;
};