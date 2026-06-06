#pragma once
#include "Dxlib.h"
#include <vector>

// ===== 当たり判定タイプ =====
enum class CollisionType
{
	Ground,
	Wall,
	Ramp
};

// ===== Box構造体 =====
struct Box
{
	// 使用モデル
	int handle = -1;

	VECTOR pos = VGet(0, 0, 0);
	VECTOR rotation = VGet(0, 0, 0);	// 回転値
	VECTOR halfSize = VGet(0, 0, 0);	// 半サイズ（x = 横幅、y = 高さ、z = 奥行）

	CollisionType type;
};

struct GroundInfo
{
	float y = 0.0f;
	VECTOR normal = VGet(0, 1, 0);
	bool isRamp = false;
};

class CollisionWorld
{
public:
	CollisionWorld();
	~CollisionWorld();

	void Init();
	void Draw();

	bool CheckWallCollision(VECTOR pos, float radius) const;
	bool CheckGroundCollision(VECTOR pos, float radius, GroundInfo& outInfo);
	bool CheckRampCollision(VECTOR pos, float radius, VECTOR& hitPos, VECTOR& hitNormal);

	int GetRampHandle() const
	{
		return rampHandle;
	}

	void DebugDraw(VECTOR playerPos);

private:
	bool SphereVsBox(VECTOR pos, float radius, const Box& box) const ;

	// ===== モデル =====
	int groundHandle;
	int wallHandle;
	int cubeHandle;
	int rampHandle;

	// ===== 地面 =====
	VECTOR groundPos;

	// ===== Box一覧 =====
	std::vector<Box> boxes;

	// ===== Ramp =====
	VECTOR rampPos;

	// デバッグ用
	int debugRampHitFlag = 0;
	VECTOR debugHitPos = VGet(0, 0, 0);

	VECTOR debugNormal = VGet(0, 0, 0);

	VECTOR rayStart;
	VECTOR rayEnd;
	bool drawRay = false;

	float hit = 0;;
	float foot = 0;
};