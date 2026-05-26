#pragma once
#include "DxLib.h"

struct WeaponData
{
	// ===== 装備位置補正 =====
	VECTOR posOffset;

	// ===== 装備回転補正 =====
	VECTOR rotOffset;

	// ===== 使用アニメ =====
	int attackAnim;
	int dashAnim;

	// ===== 攻撃判定 =====
	float attackRadius;
	float attackDistance;
	VECTOR attackOffset;

	// ===== 攻撃判定方式 =====
	bool followAttack;
};