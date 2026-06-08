#pragma once
#include "DxLib.h"

class SkyDome
{
public:
	void Init(const TCHAR* path);
	void Draw(VECTOR camPos);
	void Release();

private:
	int handle;
};