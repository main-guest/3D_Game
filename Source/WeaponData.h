#pragma once
#include "DxLib.h"
#include "MoveDirection.h"

enum class AttackShape
{
	Sphere,
	Line,
	Gun
};

/// <summary>
/// 描画専用データ
/// </summary>
struct WeaponRenderData
{
	// 装備位置
	VECTOR posOffset = VGet(0, 0, 0);

	// 装備回転
	VECTOR rotOffset = VGet(0, 0, 0);
};

struct BaseWeaponData
{
	// 攻撃位置補正
	VECTOR attackOffset = VGet(0, 0, 0);

	// 武器追従判定
	bool followAttack = false;

	// ===== 最大コンボ数 =====
	static constexpr int MaxCombo = 4;

	// ===== 攻撃 =====
	AttackShape attackShape = AttackShape::Sphere;

	// ===== 攻撃段数 =====
	int comboCount = 1;

	int attackAnim[MaxCombo] =
	{
		-1,-1,-1,-1
	};

	// ===== 判定 =====
	float attackRadius = 80.0f;
	float attackDistance = 150.0f;

	// ===== 攻撃フレーム =====
	float attackStartFrame[MaxCombo]{};
	float attackEndFrame[MaxCombo]{};

	// ===== コンボ受付 =====
	float comboAcceptStartFrame[MaxCombo]{};
	float comboAcceptEndFrame[MaxCombo]{};

	float comboNextFrame[MaxCombo]{};

	int swordDamage = 20.0f;

	int gunDamage = 30.0f;

	// ===== ダッシュ =====
	int dashAnim = -1;
};

struct WeaponData : public BaseWeaponData
{
	// ===== ロックオン歩き =====
	int lockWalkAnim[(int)MoveDirection::Count] =
	{
		-1,-1,-1,-1
	};

	// ===== ロックオンダッシュ =====
	int lockDashFrontAnim = -1;
	int lockDashBackAnim = -1;
	int lockDashRightAnim = -1;
	int lockDashLeftAnim = -1;

	// ===== ロックオン回避 =====
	int lockDodgeAnim[(int)MoveDirection::Count] =
	{
		-1,-1,-1,-1
	};

	// ===== ロックオン角度 =====
	float lockAngle = 30.0f;

	// ===== コンボキャンセル開始フレーム =====
	float comboCancelFrame[MaxCombo]{};

	float staminaCost = 20.0f;

	int damage = 20;
};

struct EnemyWeaponData : public BaseWeaponData
{
	//  ダメージ
	float attackCooldown = 1.0f;

	float staminaCost = 20.0f;

	// ===== 移動速度 =====
	float moveSpeed = 200.0f;

	float dodgeProbability = 0.5f;
	float gunDodgeProbability = 0.7f;

	float combo1Probability = 0.3f; //1段止め
	float combo2Probability = 0.4f; //2段
	float combo3Probability = 0.3f; //3段

	float dashProbability = 0.2f;

	float weaponChangeProbability = 0.1f;
};