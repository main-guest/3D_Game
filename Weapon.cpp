#include <cmath>
#include "Weapon.h"

void Weapon::Init(const TCHAR* path)
{
	handle = MV1LoadModel(path);

	MV1SetScale(handle, VGet(1.0f, 1.0f, 1.0f));
}

void Weapon::Update(int parentHandle, const TCHAR* boneName, float characterAngle)
{
	// ===== ボーン取得 =====
	int frame = MV1SearchFrame(parentHandle, boneName);

	if (frame < 0)
	{
		return;
	}

	// ===== ボーン行列取得（位置、回転、向き） =====
	MATRIX mat = MV1GetFrameLocalWorldMatrix(parentHandle, frame);

	// ===== ボーン位置 =====
	pos = VGet(
		mat.m[3][0],
		mat.m[3][1],
		mat.m[3][2]
	);

	// ===== 回転だけ取り出す =====
	// X軸
	VECTOR xAxis = VGet(mat.m[0][0], mat.m[0][1], mat.m[0][2]);

	// Y軸
	VECTOR yAxis = VGet(mat.m[1][0], mat.m[1][1], mat.m[1][2]);

	// Z軸
	VECTOR zAxis = VGet(mat.m[2][0], mat.m[2][1], mat.m[2][2]);

	// ===== 正規化 =====
	xAxis = VNorm(xAxis);
	yAxis = VNorm(yAxis);
	zAxis = VNorm(zAxis);

	// ===== ボーン基準方向 =====
	VECTOR right = xAxis;
	VECTOR up = yAxis;
	VECTOR forward = zAxis;

	// ===== 武器位置オフセット =====
	const float rightOff = 0.0f;
	const float upOff = 0.0f;
	const float fwdOff = 0.0f;

	// ===== オフセット作成 =====
	VECTOR offset;

	offset.x = right.x * rightOff + up.x * upOff + forward.x * fwdOff;
	offset.y = right.y * rightOff + up.y * upOff + forward.y * fwdOff;
	offset.z = right.z * rightOff + up.z * upOff + forward.z * fwdOff;

	// ===== 武器位置 =====
	pos = VAdd(pos, offset);

	// ===== rotMatへ代入（回転行列） =====
	MATRIX rotMat = MGetIdent();

	rotMat.m[0][0] = xAxis.x;
	rotMat.m[0][1] = xAxis.y;
	rotMat.m[0][2] = xAxis.z;

	rotMat.m[1][0] = yAxis.x;
	rotMat.m[1][1] = yAxis.y;
	rotMat.m[1][2] = yAxis.z;

	rotMat.m[2][0] = zAxis.x;
	rotMat.m[2][1] = zAxis.y;
	rotMat.m[2][2] = zAxis.z;

	// ===== 武器角度補正 =====
	MATRIX rotX = MGetRotX(DX_PI_F / -2.0f);
	MATRIX rotY = MGetRotY(DX_PI_F / 2.0f);
	MATRIX rotZ = MGetRotZ(DX_PI_F);

	// 回転合成
	MATRIX tempMat = MMult(rotX, rotY);
	MATRIX adjustMat = MMult(tempMat, rotZ);

	// ===== ボーン回転 + 武器補正 =====
	MATRIX finalMat = MMult(adjustMat, rotMat);

	// ===== 武器へ適用 =====
	MV1SetRotationMatrix(handle, finalMat);

	MV1SetPosition(handle, pos);
}

void Weapon::Draw()
{
	// ===== 武器描画 =====
	MV1DrawModel(handle);
}