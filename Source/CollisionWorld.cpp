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
	MV1SetupCollInfo(rampHandle, -1, 32, 32, 32);
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

bool CollisionWorld::CheckGroundCollision(VECTOR& pos, VECTOR& velocity, float radius, float& groundY)
{
	groundY = -100000.0f;

	bool hit = false;

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

		float footY = pos.y - radius;

		// ===== 上から接触しているか =====
		if (insideX && insideZ && footY >= top -20.0f) 
		{ 
			if (!hit || top > groundY)
			{
				groundY = top;

				hit = true;
			}

		}
	}

	// ===== Ramp判定 =====
	if (rampHandle != -1)
	{
		auto result =
			MV1CollCheck_Sphere(rampHandle, -1, pos, radius + 5.0f);

		for (int i = 0; i < result.HitNum; i++)
		{
			auto& poly = result.Dim[i];

			float slopeLimit = 0.7f; // 約45度

			// =========================
			// 床（スロープ含む）
			// =========================
			if (poly.Normal.y > slopeLimit)
			{
				HITRESULT_LINE line =
					HitCheck_Line_Triangle(
						VAdd(pos, VGet(0, 50, 0)),
						VAdd(pos, VGet(0, -100, 0)),
						poly.Position[0],
						poly.Position[1],
						poly.Position[2]
					);

				if (!line.HitFlag)
					continue;

				if (!hit || line.Position.y > groundY)
				{
					groundY = line.Position.y;
					hit = true;
				}
			}
			else
			{
				// =========================
				// 壁判定（横）
				// =========================

				VECTOR toCenter = VSub(pos, poly.Position[0]);

				float dist = VDot(toCenter, poly.Normal);

				float penetration = radius - dist;

				if (penetration > 0.0f)
				{
					pos = VAdd(pos, VScale(poly.Normal, penetration));

					float vn = VDot(velocity, poly.Normal);

					if (vn < 0.0f)
					{
						velocity = VSub(velocity, VScale(poly.Normal, vn));
					}
				}
			}
		}

		MV1CollResultPolyDimTerminate(result);
	}

	// ===== ワールド地面 =====
	if (!hit)
	{
		if (pos.y - radius <= 0.0f)
		{
			groundY = 0.0f;

			return true;
		}
	}

	return hit;
}

bool CollisionWorld::CheckRampWallCollision(VECTOR& pos, VECTOR& velocity, float radius)
{
	if (rampHandle == -1)
	{
		return false;
	}

	bool hit = false;

	auto result = MV1CollCheck_Sphere(rampHandle, -1, pos, radius + 5.0f);

	const float slopeLimit = 0.7f;

	const float wallLimit = 0.2f;

	rampDebugHit = false;
	rampHitNum = result.HitNum;

	for (int i = 0; i < result.HitNum; i++)
	{
		auto& poly = result.Dim[i];

		if (!poly.HitFlag)
			continue;

		rampDebugHit = true;

		rampTri[0] = poly.Position[0];
		rampTri[1] = poly.Position[1];
		rampTri[2] = poly.Position[2];

		rampHitPos = poly.HitPosition;
		rampNormal = poly.Normal;
		rampNormalY = poly.Normal.y;

		// ===== 床 =====
		if (poly.Normal.y > slopeLimit)
		{
			rampType = 1;
			continue;
		}

		// ===== 壁 =====
		if (fabsf(poly.Normal.y) < wallLimit)
		{
			rampType = 2;

			VECTOR diff = VSub(pos, poly.HitPosition);

			float len = VSize(diff);

			if (len < radius)
			{
				VECTOR push = VScale(VNorm(diff), radius - len + 0.5f);

				pos = VAdd(pos, push);

				float vn = VDot(velocity, VNorm(diff));

				if (vn < 0.0f)
				{
					velocity = VSub(velocity, VScale(VNorm(diff), vn));
				}

				hit = true;
			}

			continue;
		}

		// ===== 急斜面 =====
		rampType = 3;
	}

	MV1CollResultPolyDimTerminate(result);

	return hit;
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

	// 接触位置
	DrawSphere3D(
		rampHitPos,
		5.0f,
		8,
		GetColor(255, 0, 0),
		GetColor(255, 0, 0),
		TRUE);

	// 三角形
	DrawLine3D(rampTri[0], rampTri[1], GetColor(255, 255, 0));
	DrawLine3D(rampTri[1], rampTri[2], GetColor(255, 255, 0));
	DrawLine3D(rampTri[2], rampTri[0], GetColor(255, 255, 0));

	// 法線
	DrawLine3D(
		rampHitPos,
		VAdd(rampHitPos, VScale(rampNormal, 30.0f)),
		GetColor(0, 255, 0));

	DrawFormatString(
		20,
		600,
		GetColor(255, 255, 255),
		_T("HitNum : %d"),
		rampHitNum);

	DrawFormatString(
		20,
		620,
		GetColor(255, 255, 255),
		_T("NormalY : %.2f"),
		rampNormalY);

	const TCHAR* type = _T("None");

	if (rampType == 1)
		type = _T("Floor");

	if (rampType == 2)
		type = _T("Wall");

	if (rampType == 3)
		type = _T("Steep");

	DrawFormatString(
		20,
		640,
		GetColor(255, 255, 255),
		_T("Type : %s"),
		type);
}