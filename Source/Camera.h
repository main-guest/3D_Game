#pragma once
#include "Dxlib.h"

class Camera
{
public:
	Camera();
	virtual ~Camera();

	void Init();

	void Update(VECTOR playerPos);

	float GetYaw() const { return yaw; }

	int GetMouseDeltaX() const { return mouseDeltaX; }
	int GetMouseDeltaY() const { return mouseDeltaY; }

	VECTOR GetPosition() const { return cameraPos; }

private:

	float yaw;
	float pitch;

	float distance;

	VECTOR cameraPos;
	VECTOR cameraTarget;

	bool sideCamera;
	int oldF1;

	int mouseDeltaX = 0;
	int mouseDeltaY = 0;
};