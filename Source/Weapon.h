#pragma once
#include <vector>

#include "DxLib.h"
#include "WeaponData.h"

class Weapon
{
public:
	void Init(const TCHAR* path);

	void Update(int parentHandle, 
		const TCHAR* boneName, 
		float characterAngle, 
		bool removeScale);

	void Draw();

	void SetRenderData(const WeaponRenderData& d);

	void SetGlow(bool flag);

	void SetGlowColor(VECTOR color);

	const VECTOR& GetRootPosition() const
	{
		return rootPos;
	}

	const VECTOR& GetTipPosition() const
	{
		return tipPos;
	}

	const VECTOR& GetPrevRootPosition() const
	{
		return prevRootPos;
	}

	const VECTOR& GetPrevTipPosition() const
	{
		return prevTipPos;
	}

	//// 剣そのものが向いている方向
	//VECTOR GetBladeDirection() const;

	//// 前フレーム → 現在フレームの剣先移動方向
	//VECTOR GetSwingDirection() const;

	const VECTOR& GetDebugPos()const
	{
		return debugPos;
	}

	int GetHandle()const
	{
		return handle;
	}

	const MATRIX& GetWorldMatrix() const
	{
		return worldMatrix;
	}

private:
	int handle = -1;

	VECTOR pos = VGet(0, 0, 0);

	MATRIX worldMatrix = MGetIdent();

	VECTOR rootPos{};
	VECTOR tipPos{};

	VECTOR prevRootPos{};
	VECTOR prevTipPos{};

	// 剣先位置
	const float BLADE_LENGTH = 150.0f;

	WeaponRenderData renderData;

	VECTOR debugPos;

	// 発光
	bool isGlow = false;

	std::vector<int> glowMaterials;

	VECTOR glowColor = VGet(1.0f, 1.0f, 1.0f);
};