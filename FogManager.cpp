#include "FogManager.h"

void FogManager::Init()
{
	fogStart = 500.0f;
	fogEnd = 3000.0f;

	targetStart = fogStart;
	targetEnd = fogEnd;

	weatherType = 0;

	SetFogEnable(TRUE);
	SetFogColor(180, 210, 240);
}

void FogManager::Update(VECTOR camPos, VECTOR playerPos)
{
	float camHeight = camPos.y;

	// ===== 高度正規化 =====
	float heightMask = camHeight / 800.0f;
	if (heightMask < 0)heightMask = 0;
	if (heightMask > 1)heightMask = 1;

	// ===== 地面霧強度 =====
	float groundFog = 1.0f - heightMask;
	groundFog = groundFog * groundFog;

	float baseStart = 800.0f;
	float baseEnd = 4000.0f;

	// ===== 天候ベースのターゲット =====
	switch (weatherType)
	{
	case 0: // 晴れ
		baseStart = 2000.0f;
		baseEnd = 16000.0f;
		break;

	case 1: // 霧
		baseStart = 500.0f;
		baseEnd = 2500.0f;
		break;

	case 2: // ダンジョン
		baseStart = 200.0f;
		baseEnd = 1200.0f;
		break;
	}

	// ===== 層フォグ =====
	targetStart = baseStart + (300.0f * groundFog);
	targetEnd = baseEnd + (1500.0f * groundFog);

	// ===== なめらか補間 =====
	fogStart += (targetStart - fogStart) * 0.08f;
	fogEnd += (targetEnd - fogEnd) * 0.08f;

	// ===== 適用 =====
	SetFogStartEnd(fogStart, fogEnd);
}

void FogManager::SetWeather(int type)
{
	weatherType = type;
}