#include "SkyDome.h"

void SkyDome::Init(const TCHAR* path)
{
	handle = MV1LoadModel(path);
}

void SkyDome::Draw(VECTOR camPos)
{
	MV1SetPosition(handle, camPos);
	MV1SetScale(handle, VGet(1, 1, 1));

	SetUseBackCulling(FALSE);
	SetUseLighting(FALSE);
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	MV1DrawModel(handle);

	SetUseLighting(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
}

void SkyDome::Release()
{
	MV1DeleteModel(handle);
}