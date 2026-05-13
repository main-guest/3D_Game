#pragma once
#include <vector>
#include <memory>
#include "CollisionWorld.h"
#include "PhysicsManager.h"
#include "Enemy.h"

class Stage
{
public:
	void Init(CollisionWorld& world, PhysicsManager& physics);

	void Update(float dt, VECTOR playerPos);

	void Draw();

private:
	// ===== ステージオブジェクト =====
	CollisionWorld* collisionWorld = nullptr;
	PhysicsManager* physics = nullptr;

	// ===== Enemy一覧 =====
	std::vector<std::unique_ptr<Enemy>> enemies;
};