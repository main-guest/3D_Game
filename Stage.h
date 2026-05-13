#pragma once
#include <vector>
#include <memory>
#include "CollisionWorld.h"
#include "PhysicsManager.h"
#include "Enemy.h"

class Player;

class Stage
{
public:
	void Init(Player* p);

	void Update(float dt, VECTOR playerPos);

	void Draw();

	CollisionWorld& GetCollisionWorld()
	{
		return collisionWorld;
	}

	PhysicsManager& GetPhysics()
	{
		return physics;
	}

private:
	// ===== ステージオブジェクト =====
	CollisionWorld collisionWorld;
	PhysicsManager physics;

	// ===== Player参照 =====
	Player* player = nullptr;

	// ===== Enemy一覧 =====
	std::vector<std::unique_ptr<Enemy>> enemies;
};