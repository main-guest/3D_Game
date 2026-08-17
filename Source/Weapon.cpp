#include <cmath>
#include "Weapon.h"

void Weapon::Init(const TCHAR* path)
{
	handle = MV1LoadModel(path);

	MV1SetScale(handle, VGet(1.0f, 1.0f, 1.0f));

	glowMaterials.clear();

	int materialNum = MV1GetMaterialNum(handle);

	for (int i = 0; i < materialNum; i++)
	{
		const TCHAR* name = MV1GetMaterialName(handle, i);

		if (_tcscmp(name, ("Material.002")) == 0 ||
			_tcscmp(name, ("Material.027")) == 0 ||
			_tcscmp(name, ("Material.028")) == 0 ||
			_tcscmp(name, ("Material.029")) == 0 ||
			_tcscmp(name, ("Material.030")) == 0)
		{
			glowMaterials.push_back(i);
		}
	}
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
	COLOR_F color;

	if (isGlow)
	{
		color.r = glowColor.x;
		color.g = glowColor.y;
		color.b = glowColor.z;
		color.a = 1.0f;
	}
	else
	{
		color.r = 0.0f;
		color.g = 0.0f;
		color.b = 0.0f;
		color.a = 1.0f;
	}

	for (int material : glowMaterials)
	{
		MV1SetMaterialEmiColor(handle, material, color);
	}

	// ===== 武器描画 =====
	MV1DrawModel(handle);
}

VECTOR Weapon::GetMuzzlePosition(const VECTOR& offset) const
{
	// 銃口方向を180度反転
	MATRIX rotMat = MGetRotY(DX_PI_F);

	// オフセットを180度回転
	VECTOR rotOffset = VTransform(offset, rotMat);

	// 武器のワールド座標へ変換
	return VTransform(rotOffset, worldMatrix);
}

VECTOR Weapon::GetMuzzleDirection() const
{
	// 武器のローカルZ軸を前方向とする
	VECTOR dir = VGet(
		worldMatrix.m[2][0],
		worldMatrix.m[2][1],
		worldMatrix.m[2][2]
	);

	// 正規化
	float length = VSize(dir);

	if (length > 0.0001f)
	{
		dir = VScale(dir, 1.0f / length);
	}

	return dir;
}

void Weapon::SetRenderData(const WeaponRenderData& d)
{
	renderData = d;
}

void Weapon::SetGlow(bool flag)
{
	isGlow = flag;
}

void Weapon::SetGlowColor(VECTOR color)
{
	glowColor = color;
}