#include "PhysicsManager.h"
#include "CollisionWorld.h"

void PhysicsManager::Init(CollisionWorld* w)
{
	world = w;
}

bool PhysicsManager::MoveAndCheckCollision(VECTOR& pos, VECTOR& velocity, float radius, bool& isGround, float dt)
{
	// ===== d—Í =====
	velocity.y -= gravity * dt;

	// ===== ˆÚ“®ˆ— =====
	// X•ûŒü
	VECTOR nextPos = pos;
	nextPos.x += velocity.x * dt;

	if (!world->CheckWallCollision(nextPos, radius))
	{
		pos.x = nextPos.x;
	}

	// Z•ûŒü
	nextPos = pos;
	nextPos.z += velocity.z * dt;

	if (!world->CheckWallCollision(nextPos, radius))
	{
		pos.z = nextPos.z;
	}

	// Y•ûŒü
	nextPos = pos;
	nextPos.y += velocity.y * dt;

	float groundY = 0.0f;

	if (velocity.y > 0.0f)
	{
		pos.y = nextPos.y;
		isGround = false;
	}
	else
	{
		// —Ž‰º’†‚Ì‚ÝÚ’n
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

	// ===== ’n–Ê =====
	if (pos.y < groundHeight)
	{
		pos.y = groundHeight;

		velocity.y = 0.0f;

		isGround = true;
	}

	return true;
}