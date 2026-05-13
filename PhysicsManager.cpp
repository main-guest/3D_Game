#include <cmath>
#include "PhysicsManager.h"
#include "CollisionWorld.h"

void PhysicsManager::Init(CollisionWorld* w)
{
	world = w;
}

void PhysicsManager::MoveCharacter(VECTOR& pos, VECTOR& velocity, float radius, bool& isGround, float dt)
{
	// ===== 重力 =====
	velocity.y -= gravity * dt;

	// ===== 移動処理 =====
	// X方向
	VECTOR nextPos = pos;
	nextPos.x += velocity.x * dt;

	if (!world->CheckWallCollision(nextPos, radius))
	{
		pos.x = nextPos.x;
	}

	// Z方向
	nextPos = pos;
	nextPos.z += velocity.z * dt;

	if (!world->CheckWallCollision(nextPos, radius))
	{
		pos.z = nextPos.z;
	}

	// Y方向
	nextPos = pos;
	nextPos.y += velocity.y * dt;

	float groundY;

	if (velocity.y > 0.0f)
	{
		pos.y = nextPos.y;
		isGround = false;
	}
	else
	{
		// 落下中のみ接地
		if (!world->CheckGroundCollision(nextPos, radius, groundY))
		{
			pos.y = nextPos.y;
			isGround = false;
		}
		else
		{
			isGround = true;
			velocity.y = 0.0f;
			pos.y = groundY + radius;
		}
	}
}

void PhysicsManager::ResolveCharacterCollision(VECTOR& pos, VECTOR& velocity, float radius, const std::vector<VECTOR>& others)
{
	for (const auto& o : others)
	{
		float dx = pos.x - o.x;
		float dz = pos.z - o.z;

		float distSq = dx * dx + dz * dz;
		float minDist = radius * 10.0f; // 同サイズ想定

		if (distSq < minDist * minDist)
		{
			float dist = sqrtf(distSq);
			if (dist < 0.001f) continue;

			// 押し出し方向
			float pushX = dx / dist;
			float pushZ = dz / dist;

			float overlap = (minDist - dist);

			// ===== 位置補正（半分ずつ）=====
			pos.x += pushX * overlap * 0.5f;
			pos.z += pushZ * overlap * 0.5f;

			// ===== 速度も殺す =====
			float dot = velocity.x * pushX + velocity.z * pushZ;

			if (dot < 0.0f)
			{
				velocity.x -= pushX * dot;
				velocity.z -= pushZ * dot;
			}
		}
	}
}