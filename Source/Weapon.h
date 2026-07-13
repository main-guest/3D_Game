#pragma once
#include "DxLib.h"
#include "WeaponData.h"

class Weapon
{
public:
	void Init(const TCHAR* path);

	void Update(int parentHandle, const TCHAR* boneName, float characterAngle, bool removeScale = false);

	void Draw();

	void SetRenderData(const WeaponRenderData& d);

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

	// åïêÊà íu
	const float BLADE_LENGTH = 150.0f;

	WeaponRenderData renderData;

	VECTOR debugPos;
};