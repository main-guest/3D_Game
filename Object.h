#pragma once
#include "Dxlib.h"
#include <vector>

// ===== Box構造体 =====
struct Box
{
	// 使用モデル
	int handle;

	VECTOR pos;
	VECTOR rotation;	// 回転値
	VECTOR halfSize;	// 半サイズ（x = 横幅、y = 高さ、z = 奥行）

};

class Object
{
public:
	Object();
	virtual ~Object();

	void Init();
	void Draw();

	bool CheckCollision(VECTOR playerPos, float radius);

private:
	// ===== モデル =====
	int groundHandle;
	int wallHandle;
	int cubeHandle;

	// ===== 地面 =====
	VECTOR groundPos;

	// ===== Box一覧 =====
	std::vector<Box> boxes;
};