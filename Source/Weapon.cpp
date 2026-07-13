#include <cmath>
#include "Weapon.h"

void Weapon::Init(const TCHAR* path)
{
	handle = MV1LoadModel(path);

	MV1SetScale(handle, VGet(1.0f, 1.0f, 1.0f));
}

void Weapon::Update(int parentHandle, const TCHAR* boneName, float characterAngle, bool removeScale)
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

	// ===== Enemy用：ボーンスケール除去 =====
	if (removeScale)
	{
		VECTOR xAxis =
		{
			handMat.m[0][0],
			handMat.m[0][1],
			handMat.m[0][2]
		};

		VECTOR yAxis =
		{
			handMat.m[1][0],
			handMat.m[1][1],
			handMat.m[1][2]
		};

		VECTOR zAxis =
		{
			handMat.m[2][0],
			handMat.m[2][1],
			handMat.m[2][2]
		};

		float sx = VSize(xAxis);
		float sy = VSize(yAxis);
		float sz = VSize(zAxis);

		if (sx > 0.0001f)
		{
			handMat.m[0][0] /= sx;
			handMat.m[0][1] /= sx;
			handMat.m[0][2] /= sx;
		}

		if (sy > 0.0001f)
		{
			handMat.m[1][0] /= sy;
			handMat.m[1][1] /= sy;
			handMat.m[1][2] /= sy;
		}

		if (sz > 0.0001f)
		{
			handMat.m[2][0] /= sz;
			handMat.m[2][1] /= sz;
			handMat.m[2][2] /= sz;
		}
	}

	// デバッグ座標
	debugPos = VGet(handMat.m[3][0], handMat.m[3][1], handMat.m[3][2]);

	// ===== 武器位置オフセット =====
	MATRIX transMat = MGetTranslate(renderData.posOffset);

	// ===== 武器回転オフセット =====
	// X軸
	MATRIX rotX = MGetRotX(renderData.rotOffset.x);

	// Y軸
	MATRIX rotY = MGetRotY(renderData.rotOffset.y);

	// Z軸
	MATRIX rotZ = MGetRotZ(renderData.rotOffset.z);

	MATRIX rotOffset = MMult(MMult(rotX, rotY), rotZ);

	// ===== オフセット行列 =====
	MATRIX offsetMat = MMult(rotOffset, transMat);

	// ===== 武器最終行列 =====
	MATRIX weaponMat = MMult(offsetMat, handMat);

	// 保存
	worldMatrix = weaponMat;

	
	// ===== 武器へ適用 =====
	MV1SetMatrix(handle, weaponMat);

	// 現在フレーム更新
	rootPos = VTransform(VGet(0.0f, 0.0f, 0.0f), worldMatrix);
	tipPos = VTransform(VGet(0.0f, BLADE_LENGTH, 0.0f), worldMatrix);
}

void Weapon::Draw()
{
	// ===== 武器描画 =====
	MV1DrawModel(handle);
}

void Weapon::SetRenderData(const WeaponRenderData& d)
{
	renderData = d;
}