#include "TerrainSystem.h"

void TerrainSystem::Init()
{
	baseHeight = 0.0f;
}

float TerrainSystem::GetGroundHeight(VECTOR pos) const
{
	// 坂（Slope）
	float slopeHeight = pos.z * slopeFactor;

	// 階段（Step）
	int step = (int)(pos.z / stepDepth);
	float stepHeightValue = step * stepHeight;

	// 合成
	float height = baseHeight + slopeHeight + stepHeightValue;

	return height;
}

VECTOR TerrainSystem::GetGroundNormal(VECTOR pos) const
{
	float hL = GetGroundHeight(VAdd(pos, VGet(-1, 0, 0)));
	float hR = GetGroundHeight(VAdd(pos, VGet(1, 0, 0)));
	float hD = GetGroundHeight(VAdd(pos, VGet(0, 0, -1)));
	float hU = GetGroundHeight(VAdd(pos, VGet(0, 0, 1)));

	VECTOR n;

	n.x = hL - hR;
	n.y = 2.0f;
	n.z = hD - hU;

	return VNorm(n);
}