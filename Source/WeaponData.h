#pragma once
#include "DxLib.h"

enum class AttackShape
{
	Sphere,
	Line,
	Gun
};

struct WeaponData
{
	// ===== 使用アニメ =====
	int attackAnim;
	int dashAnim;

	AttackShape attackShape;

	// ===== 攻撃判定 =====
	float attackRadius;
	float attackDistance;

	VECTOR attackOffset;

	// ===== 装備位置補正 =====
	VECTOR posOffset;

	// ===== 装備回転補正 =====
	VECTOR rotOffset;

	// ===== 攻撃判定方式 =====
	bool followAttack;

	// ===== ロックオン角度 =====
	float lockAngle = 30.0f;

	// ===== 判定開始・終了フレーム =====
	float attackStartFrame;
	float attackEndFrame;
};