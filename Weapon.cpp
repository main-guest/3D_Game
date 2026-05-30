#include <cmath>
#include "Weapon.h"

void Weapon::Init(const TCHAR* path)
{
	handle = MV1LoadModel(path);

	MV1SetScale(handle, VGet(1.0f, 1.0f, 1.0f));
}

void Weapon::Update(int parentHandle, const TCHAR* boneName, float characterAngle)
{
	// 前フレーム保存
	prevRootPos = rootPos;
	prevTipPos = tipPos;

	// ===== ボーン取得 =====
	int frame = MV1SearchFrame(parentHandle, boneName);

	if (frame < 0)
	{
		return;
	}

	// ===== 手ボーン行列取得（位置、回転、向き） =====
	MATRIX handMat = MV1GetFrameLocalWorldMatrix(parentHandle, frame);

	// ===== 武器位置オフセット =====
	MATRIX transMat = MGetTranslate(data.posOffset);

	// ===== 武器回転オフセット =====
	// X軸
	MATRIX rotX = MGetRotX(data.rotOffset.x);

	// Y軸
	MATRIX rotY = MGetRotY(data.rotOffset.y);

	// Z軸
	MATRIX rotZ = MGetRotZ(data.rotOffset.z);

	MATRIX rotOffset = MMult(MMult(rotX, rotY), rotZ);

	// ===== オフセット行列 =====
	MATRIX offsetMat = MMult(rotOffset, transMat);

	// ===== 武器最終行列 =====
	MATRIX weaponMat = MMult(offsetMat, handMat);

	// 保存
	worldMatrix = weaponMat;

	
	// ===== 武器へ適用 =====
	MV1SetMatrix(handle, weaponMat);

	// デバッグ座標
	debugPos = VGet(weaponMat.m[3][0], weaponMat.m[3][1], weaponMat.m[3][2]);

	// 現在フレーム更新
	rootPos = VTransform(VGet(0.0f, 0.0f, 0.0f), worldMatrix);
	tipPos = VTransform(VGet(0.0f, BLADE_LENGTH, 0.0f), worldMatrix);
}

void Weapon::Draw()
{
	// ===== 武器描画 =====
	MV1DrawModel(handle);
}

void Weapon::SetData(const WeaponData& d)
{
	data = d;
}