#include <cmath>
#include "PhysicsManager.h"
#include "CollisionWorld.h"

void PhysicsManager::Init(CollisionWorld* w)
{
	world = w;
}

bool PhysicsManager::MoveCharacter(VECTOR& pos, VECTOR& velocity, float radius, bool& isGround, float dt)
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

	float groundY = 0.0f;

	if (!world->CheckGroundCollision(nextPos, radius, groundY))
	{
		pos.y = nextPos.y;
		isGround = false;
	}
	else
	{
		pos.y = groundY + radius;

		velocity.y = 0.0f;

		isGround = true;
	}

	return true;
}

void PhysicsManager::ResolveCharacterCollision(VECTOR& pos, VECTOR& velocity, float radius, float height, const std::vector<VECTOR>& others, float otherHeight)
{
	const float EPS = 0.0001f;

	for (int iter = 0; iter < 3; iter++)
	{
		// 自分のカプセル
		VECTOR A = pos;					// 足元
		VECTOR B = VGet(pos.x, pos.y + height, pos.z);	// 頭

		bool anyHit = false;

		for (const auto& o : others)
		{
			// 相手カプセル
			VECTOR OA = o;
			VECTOR OB = VGet(o.x, o.y + otherHeight, o.z);

			// ===== 最近接点 =====
			VECTOR p1 = ClosestPointOnSegment(A, B, o);
			VECTOR p2 = ClosestPointOnSegment(OA, OB, pos);

			float dx = p1.x - p2.x;
			float dy = p1.y - p2.y;
			float dz = p1.z - p2.z;

			float distSq = dx * dx + dy * dy + dz * dz;

			float minDist = radius * radius;

			if (distSq < minDist * minDist)
			{
				float dist = sqrtf(distSq);
				if (dist < EPS)continue;

				float nx = dx / dist;
				float ny = dy / dist;
				float nz = dz / dist;

				// ===== 押し出し =====
				float overlap = (minDist - dist) + 0.001f;

				pos.x += nx * overlap;
				pos.y += ny * overlap;
				pos.z += nz * overlap;

				// ===== 速度減衰 =====
				float vn = velocity.x * nx + velocity.y * ny + velocity.z * nz;

				if (vn < 0.0f)
				{
					velocity.x -= nx * vn;
					velocity.y -= ny * vn;
					velocity.z -= nz * vn;
				}

				anyHit = true;
			}

			if (!anyHit) break;
		}
	}
}

float PhysicsManager::Dot(VECTOR a, VECTOR b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

VECTOR PhysicsManager::Sub(VECTOR a, VECTOR b)
{
	return VGet(a.x - b.x, a.y - b.y, a.z - b.z);
}

VECTOR PhysicsManager::Add(VECTOR a, VECTOR b)
{
	return VGet(a.x + b.x, a.y + b.y, a.z + b.z);
}

VECTOR PhysicsManager::Mul(VECTOR v, float f)
{
	return VGet(v.x * f, v.y * f, v.z * f);
}

VECTOR PhysicsManager::ClosestPointOnSegment(VECTOR a, VECTOR b, VECTOR p)
{
	VECTOR ab = Sub(b, a);
	VECTOR ap = Sub(p, a);

	float abLenSq = Dot(ab, ab);
	if (abLenSq < 0.0001f)return a;

	float t = Dot(ap, ab) / abLenSq;

	if (t < 0.0f)t = 0.0f;
	if (t > 1.0f)t = 1.0f;

	return Add(a, Mul(ab, t));
}