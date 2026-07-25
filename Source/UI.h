#pragma once

class Player;
class Enemy;

class UI
{
public:
	void Init(Player* player);

	void Update(float dt, Player* player);

	void Draw(Player* player);

	void UpdatePlayer(float dt, Player* player);

	void DrawPlayer();

	void SetEnemy(Enemy* enemy);

	void UpdateEnemy(float dt);

	void DrawEnemy();

	void DrawLockOn(Player* player);

private:
	// ===== PLAYER =====
	// HPバー
	float displayHp = 0.0f;

	float maxHp = 0.0f;

	// STバー
	float displaySt = 0.0f;

	float maxSt = 0.0f;

	// 遅延バー
	float delayHp;

	float damageDelayTimer;

	// ===== Enemy =====
	float enemyDisplayHp = 0.0f;
	float enemyDelayHp = 0.0f;
	float enemyMaxHp = 0.0f;

	float enemyDamageDelayTimer = 0.0f;

	// 表示対象
	Enemy* bossEnemy = nullptr;
};