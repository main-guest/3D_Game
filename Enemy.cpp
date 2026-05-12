#include "Enemy.h"
#include <cmath>

void Enemy::Update(Object& object, VECTOR playerPos)
{
	VECTOR dir;

	dir.x = playerPos.x - pos.x;
	dir.z = playerPos.z - pos.z;

	float len = sqrtf(dir.x * dir.x + dir.z * dir.z);

	if (len > 0.0f)
	{
		dir.x /= len;
		dir.z /= len;

		pos.x += dir.x * speed;
		pos.z += dir.z * speed;

		characterAngle = atan2f(dir.x, dir.z) + DX_PI;

		currentState = AnimState::Walk;
	}
	else
	{
		currentState = AnimState::Idle;
	}

	UpdateGravity(object);

	UpdateAnimation();
}