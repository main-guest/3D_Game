#include "CollisionWorld.h"

CollisionWorld::CollisionWorld()
	:groundHandle(-1),
	wallHandle(-1),
	cubeHandle(-1),
	rampHandle(-1),
	rampHit(false),
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

	if (rampHandle != -1)
	{
		MV1DeleteModel(rampHandle);
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
	groundHandle = MV1LoadModel(_T("Assets/mv1model/asphalt.mv1"));
	wallHandle = MV1LoadModel(_T("Assets/mv1model/wall.mv1"));
	cubeHandle = MV1LoadModel(_T("Assets/mv1model/cube.mv1"));
	rampHandle = MV1LoadModel(_T("Assets/mv1model/Ramp.mv1"));

	// ===== 地面 =====
	{
		Box ground;

		ground.handle = MV1DuplicateModel(groundHandle);

		ground.pos = groundPos;
		ground.rotation = VGet(0, 0, 0);
		ground.halfSize = VGet(10000, 0, 10000);

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

	// ==================================================
	// Ramp
	// ==================================================
	rampPos = VGet(0, 0, 1200);

	// Rampモデル位置
	MV1SetPosition(rampHandle, rampPos);
	MV1SetRotationXYZ(rampHandle, VGet(0, 0, 0));

	// ポリゴンコリジョン構築
	MV1SetupCollInfo(rampHandle, -1, 8, 8, 8);
}

void CollisionWorld::Draw()
{
	// ===== Box描画 =====
	for (auto& box : boxes)
	{
		MV1SetPosition(box.handle, box.pos);

		MV1SetRotationXYZ(box.handle, box.rotation);

		MV1DrawModel(box.handle);
	}

	// ===== Ramp描画 =====
	MV1DrawModel(rampHandle);
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

		// ===== Box上面 =====
		float top = box.pos.y + box.halfSize.y;

		// ===== 上に乗っているなら壁判定しない =====
		if (pos.y - radius >= top - 5.0f)
		{
			continue;
		}

		// ===== 通常壁判定 =====
		if (SphereVsBox(pos, radius, box))
		{
			return true;
		}
	}

	return false;
}

bool CollisionWorld::CheckGroundCollision(VECTOR pos, float radius, float& groundY)
{
	for (const auto& box : boxes)
	{
		if (box.type != CollisionType::Ground && box.type != CollisionType::Wall)
		{
			continue;
		}

		// ===== Boxの上面 =====
		float top = box.pos.y + box.halfSize.y;

		// ===== XZ範囲内か =====
		bool insideX =
			pos.x >= box.pos.x - box.halfSize.x &&
			pos.x <= box.pos.x + box.halfSize.x;

		bool insideZ =
			pos.z >= box.pos.z - box.halfSize.z &&
			pos.z <= box.pos.z + box.halfSize.z;

		// ===== 上から接触しているか =====
		bool onTop = pos.y - radius <= top && pos.y > top;

		if (insideX && insideZ && onTop) 
		{ 
			groundY = top; return true;
		}
	}

	// ===== Ramp判定 =====
	if (rampHandle != -1)
	{
		VECTOR start = pos;

		VECTOR end = VSub(pos, VGet(0.0f, 1000.0f, 0.0f));

		MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(rampHandle, -1, start, end);

		if (result.HitFlag)
		{
			// ヒット位置を保存
			rampHitPos = result.HitPosition;
			rampHit = true;

			DrawLine3D(
				start,
				end,
				GetColor(0, 255, 0)
			);

			groundY = result.HitPosition.y;

			return true;
		}
	}

	// ===== ワールド地面 =====
	if (pos.y - radius <= 0.0f)
	{
		groundY = 0.0f;

		return true;
	}

	return false;
}

void CollisionWorld::DebugDraw()
{
	if (rampHit)
	{
		DrawSphere3D(
			rampHitPos,
			10.0f,
			8,
			GetColor(255, 0, 0),
			GetColor(255, 0, 0),
			TRUE
		);
	}
}