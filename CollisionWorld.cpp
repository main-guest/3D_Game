#include "CollisionWorld.h"

CollisionWorld::CollisionWorld()
	:groundHandle(-1),
	wallHandle(-1),
	cubeHandle(-1),
	groundPos(VGet(0, 0, 0))
{

}

CollisionWorld::~CollisionWorld()
{
	if (groundHandle != -1)
	{
		MV1DeleteModel(groundHandle);
	}

	if (wallHandle != -1)
	{
		MV1DeleteModel(wallHandle);
	}

	if (cubeHandle != -1)
	{
		MV1DeleteModel(cubeHandle);
	}

	for (auto& box : boxes)
	{
		if (box.handle != -1)
		{
			MV1DeleteModel(box.handle);
		}
	}
}

void CollisionWorld::Init()
{
	// ===== モデル読み込み =====
	groundHandle = MV1LoadModel(_T("mv1model/asphalt.mv1"));
	wallHandle = MV1LoadModel(_T("mv1model/wall.mv1"));
	cubeHandle = MV1LoadModel(_T("mv1model/cube.mv1"));

	// ===== 地面 =====
	{
		Box ground;

		ground.handle = MV1DuplicateModel(groundHandle);

		ground.pos = VGet(0, 0, 0);
		ground.rotation = VGet(0, 0, 0);
		ground.halfSize = VGet(1000, 10, 1000);

		ground.type = CollisionType::Ground;

		boxes.push_back(ground);
	}

	// ==================================================
	// 横壁
	// ==================================================
	{
		Box wall;
		
		wall.handle = MV1DuplicateModel(wallHandle);

		wall.pos = VGet(500, 0, 0);
		wall.rotation = VGet(0, 0, 0);
		wall.halfSize = VGet(220, 300, 40);

		wall.type = CollisionType::Wall;

		boxes.push_back(wall);
	}

	{
		Box wall;

		wall.handle = MV1DuplicateModel(wallHandle);

		wall.pos = VGet(-500, 0, 0);
		wall.rotation = VGet(0, 0, 0);
		wall.halfSize = VGet(220, 300, 40);

		wall.type = CollisionType::Wall;

		boxes.push_back(wall);
	}

	// ==================================================
	// 縦壁（90度回転）
	// ==================================================
	{
		Box wall;

		wall.handle = MV1DuplicateModel(wallHandle);
		
		wall.pos = VGet(0, 0, 500);
		wall.rotation = VGet(0, DX_PI_F / 2.0f, 0);
		wall.halfSize = VGet(40, 300, 220);

		wall.type = CollisionType::Wall;

		boxes.push_back(wall);
	}

	{
		Box wall;

		wall.handle = MV1DuplicateModel(wallHandle);

		wall.pos = VGet(0, 0, -500);
		wall.rotation = VGet(0, DX_PI_F / 2.0f, 0);
		wall.halfSize = VGet(40, 300, 220);

		wall.type = CollisionType::Wall;

		boxes.push_back(wall);
	}

	// ==================================================
	// Cube
	// ==================================================
	{
		Box cube;

		cube.handle = MV1DuplicateModel(cubeHandle);

		cube.pos = VGet(400, 0, 150);
		cube.rotation = VGet(0, 0, 0);
		cube.halfSize = VGet(70, 70, 70);


		cube.type = CollisionType::Wall;

		boxes.push_back(cube);
	}
}

void CollisionWorld::Draw()
{
	// ===== 地面 =====
	MV1SetPosition(groundHandle, groundPos);
	MV1DrawModel(groundHandle);

	// ===== Box描画 =====
	for (auto& box : boxes)
	{
		MV1SetPosition(box.handle, box.pos);

		MV1SetRotationXYZ(box.handle, box.rotation);

		MV1DrawModel(box.handle);
	}
}

bool CollisionWorld::SphereVsBox(VECTOR pos, float radius, const Box& box) const
{
	// ===== Box範囲 =====
	float left = box.pos.x - box.halfSize.x;
	float right = box.pos.x + box.halfSize.x;

	float bottom = box.pos.y - box.halfSize.y;
	float top = box.pos.y + box.halfSize.y;

	float front = box.pos.z - box.halfSize.z;
	float back = box.pos.z + box.halfSize.z;

	// ===== 最も近い点 =====
	float closestX = pos.x;
	float closestY = pos.y;
	float closestZ = pos.z;

	// clamp X
	if (closestX < left) closestX = left;
	if (closestX > right) closestX = right;

	// clamp Y
	if (closestY < bottom) closestY = bottom;
	if (closestY > top) closestY = top;

	// clamp Z
	if (closestZ < front) closestZ = front;
	if (closestZ > back) closestZ = back;

	// ===== 距離 =====
	float dx = pos.x - closestX;
	float dy = pos.y - closestY;
	float dz = pos.z - closestZ;

	float distanceSq =
		dx * dx +
		dy * dy +
		dz * dz;

	return distanceSq<=radius*radius;
}

bool CollisionWorld::CheckWallCollision(VECTOR pos, float radius) const
{
	for (const auto& box : boxes)
	{
		if (box.type != CollisionType::Wall)
		{
			continue;
		}

		if (SphereVsBox(pos, radius, box))
		{
			return true;
		}
	}

	return false;
}

bool CollisionWorld::CheckGroundCollision(VECTOR pos, float radius, float& groundY) const
{
	for (const auto& box : boxes)
	{
		if (box.type != CollisionType::Ground)
		{
			continue;
		}

		if (SphereVsBox(pos, radius, box))
		{
			// Box上面
			groundY = box.pos.y + box.halfSize.y;

			return true;
		}
	}

	if (pos.y - radius <= 0.0f)
	{
		groundY = 0.0f;

		return true;
	}

	return false;
}