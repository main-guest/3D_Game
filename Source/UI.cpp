#include "DxLib.h"
#include "UI.h"
#include "Player.h"
#include "Enemy.h"
#include "ScreenSize.h"

void UI::Init(Player* player)
{
	displayHp = player->GetMaxHp();
	delayHp = player->GetMaxHp();

	displaySt = player->GetMaxStamina();

	damageDelayTimer = 0.0f;
}

void UI::Update(float dt, Player* player)
{
	UpdatePlayer(dt, player);

	UpdateEnemy(dt);
}


void UI::Draw(Player* player)
{
	DrawPlayer();

	DrawEnemy();

	DrawLockOn(player);
}


void UI::UpdatePlayer(float dt, Player* player)
{
	// === HP ===
	float hp = player->GetHp();
	maxHp = player->GetMaxHp();

	if (displayHp != hp)
	{
		displayHp = hp;
		damageDelayTimer = 0.5f;
	}

	if (damageDelayTimer > 0)
	{
		damageDelayTimer -= dt;
	}
	else
	{
		if (delayHp > hp)
		{
			delayHp -= 30.0f * dt;

			if (delayHp < hp)
			{
				delayHp = hp;
			}
		}
	}

	// === ST ===
	float st = player->GetStamina();
	maxSt = player->GetMaxStamina();

	if (displaySt != st)
	{
		displaySt = st;
	}
}

void UI::DrawPlayer()
{
	// ===== HP =====
	float hpRate = displayHp / maxHp;
	float delayRate = delayHp / maxHp;

	int width = 300;
	int hpX = 20;
	int hpY = 20;

	// --- ”wŒi ---
	DrawBox(hpX, hpY,
		hpX + width,
		hpY + 30,
		GetColor(50, 50, 50), TRUE);

	// --- HP’x‰„ƒQ[ƒW ---
	DrawBox(hpX, hpY,
		hpX + (int)(width * delayRate),
		hpY + 30,
		GetColor(255, 0, 0), TRUE);

	// --- HPƒQ[ƒW ---
	DrawBox(hpX, hpY,
		hpX + (int)(width * hpRate),
		hpY + 30,
		GetColor(0, 255, 0), TRUE);

	// --- ˜g ---
	DrawBox(hpX - 2, hpY - 2,
		hpX + width + 2,
		hpY + 30 + 2,
		GetColor(0, 0, 0), FALSE);

	DrawString(hpX + 5, hpY + 5,
		"HP",
		GetColor(0, 0, 0));

	// ===== STAMINA =====
	float staminaRate = displaySt / maxSt;

	int stX = 20;
	int stY = 60;

	DrawBox(stX, stY,
		stX + width,
		stY + 30,
		GetColor(50, 50, 50), TRUE);

	// --- ƒQ[ƒW ---
	DrawBox(stX, stY,
		stX + (int)(width * staminaRate),
		stY + 30,
		GetColor(255, 140, 0), TRUE);

	// --- ˜g ---
	DrawBox(stX - 2, stY - 2,
		stX + width + 2,
		stY + 32,
		GetColor(0, 0, 0), FALSE);

	DrawString(stX + 5, stY + 5,
		"ST",
		GetColor(0, 0, 0));
}

void UI::SetEnemy(Enemy* enemy)
{
	bossEnemy = enemy;

	if (bossEnemy)
	{
		enemyMaxHp = bossEnemy->GetMaxHp();

		enemyDisplayHp = enemyMaxHp;
		enemyDelayHp = enemyMaxHp;
	}
}

void UI::UpdateEnemy(float dt)
{
	if (!bossEnemy)
		return;

	// Boss‚ªŽ€–S‚µ‚½‚ç•\Ž¦‰ðœ
	if (bossEnemy->IsDead())
	{
		bossEnemy = nullptr;
		return;
	}

	float hp = bossEnemy->GetHp();

	if (enemyDisplayHp != hp)
	{
		enemyDisplayHp = hp;
		enemyDamageDelayTimer = 0.5f;
	}

	if (enemyDamageDelayTimer > 0)
	{
		enemyDamageDelayTimer -= dt;
	}
	else
	{
		if (enemyDelayHp > hp)
		{
			enemyDelayHp -= 50.0f * dt;

			if (enemyDelayHp < hp)
			{
				enemyDelayHp = hp;
			}
		}
	}
}

void UI::DrawEnemy()
{
	if (!bossEnemy)
		return;

	int width = 800;
	int x = (1280 - width) / 2;
	int y = 650;

	float hpRate = enemyDisplayHp / enemyMaxHp;

	float delayRate = enemyDelayHp / enemyMaxHp;

	// --- ”wŒi ---
	DrawBox(x, y,
		x + width,
		y + 30,
		GetColor(40, 40, 40), TRUE);

	// --- ’x‰„ ---
	DrawBox(x, y,
		x + (int)(width * delayRate),
		y + 30,
		GetColor(255, 0, 0), TRUE);

	// --- HP ---
	DrawBox(x, y,
		x + (int)(width * hpRate),
		y + 30,
		GetColor(0, 200, 0), TRUE);

	// --- ˜g ---
	DrawBox(x - 2, y - 2,
		x + width + 2,
		y + 32,
		GetColor(0, 0, 0), FALSE);

	DrawString(x, y - 20,
		"Enemy",
		GetColor(0, 0, 0));
}

void UI::DrawLockOn(Player* player)
{
	Enemy* enemy = player->GetLockOnTarget();

	if (!enemy)
		return;

	if (enemy->IsDead())
		return;

	VECTOR pos = enemy->GetCenterPos();

	VECTOR screen = ConvWorldPosToScreenPos(pos);

	int x = (int)screen.x;
	int y = (int)screen.y;

	DrawCircle(x, y,
		3,
		GetColor(255, 255, 255),
		TRUE);
}