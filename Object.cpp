#include "Object.h"

Object::Object()
	:groundHandle(-1),
	wallHandle(-1),
	cubeHandle(-1)
{

}

Object::~Object()
{
	MV1DeleteModel(groundHandle);
	MV1DeleteModel(wallHandle);
	MV1DeleteModel(cubeHandle);

	// 複製モデル削除
	for (auto& box : boxes)
	{
		MV1DeleteModel(box.handle);
	}
}

void Object::Init()
{
	// ===== モデル読み込み =====
	groundHandle = MV1LoadModel(_T("mv1model/asphalt.mv1"));
	wallHandle = MV1LoadModel(_T("mv1model/wall.mv1"));
	cubeHandle = MV1LoadModel(_T("mv1model/cube.mv1"));

	// ===== 地面 =====
	groundPos = VGet(0, 0, 0);

	// ==================================================
	// 横壁
	// ==================================================
	{
		Box wall;
		
		wall.handle = MV1DuplicateModel(wallHandle);

		wall.pos = VGet(300, 0, 0);
		wall.rotation = VGet(0, 0, 0);
		wall.halfSize = VGet(220, 300, 40);

		boxes.push_back(wall);
	}

	{
		Box wall;

		wall.handle = MV1DuplicateModel(wallHandle);

		wall.pos = VGet(-300, 0, 0);
		wall.rotation = VGet(0, 0, 0);
		wall.halfSize = VGet(220, 300, 40);

		boxes.push_back(wall);
	}

	// ==================================================
	// 縦壁（90度回転）
	// ==================================================
	{
		Box wall;

		wall.handle = MV1DuplicateModel(wallHandle);
		
		wall.pos = VGet(0, 0, 300);
		wall.rotation = VGet(0, DX_PI_F / 2.0f, 0);
		wall.halfSize = VGet(40, 300, 220);

		boxes.push_back(wall);
	}

	{
		Box wall;

		wall.handle = MV1DuplicateModel(wallHandle);

		wall.pos = VGet(0, 0, -300);
		wall.rotation = VGet(0, DX_PI_F / 2.0f, 0);
		wall.halfSize = VGet(40, 300, 220);

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

		boxes.push_back(cube);
	}
}

void Object::Draw()
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

bool Object::CheckCollision(VECTOR playerPos, float radius)
{
	for (const auto& box : boxes)
	{
		// ===== Box範囲 =====
		float left = box.pos.x - box.halfSize.x;
		float right = box.pos.x + box.halfSize.x;

		float bottom = box.pos.y - box.halfSize.y;
		float top = box.pos.y + box.halfSize.y;

		float front = box.pos.z - box.halfSize.z;
		float back = box.pos.z + box.halfSize.z;

		// ===== 最も近い点 =====
		float closestX = playerPos.x;
		float closestY = playerPos.y;
		float closestZ = playerPos.z;

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
		float dx = playerPos.x - closestX;
		float dy = playerPos.y - closestY;
		float dz = playerPos.z - closestZ;

		float distanceSq =
			dx * dx +
			dy * dy +
			dz * dz;

		// ===== Sphere vs Box =====
		if (distanceSq < radius * radius)
		{
			return true;
		}
	}

	return false;
}
