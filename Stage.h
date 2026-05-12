#pragma once
#include <vector>
#include <memory>
#include "Object.h"
#include "Enemy.h"

class Stage
{
public:
	Stage();
	~Stage();

	void Init();

	void Update(float deltaTime, VECTOR playerPos);

	void Draw();

	Object& GetObject()
	{
		return object;
	}

private:
	// ===== ステージオブジェクト =====
	Object object;

	// ===== Enemy一覧 =====
	std::vector<std::unique_ptr<Enemy>> enemies;
};