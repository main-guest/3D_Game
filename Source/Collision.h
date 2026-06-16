#pragma once
#include "DxLib.h"

bool LineCapsuleHit(
	const VECTOR& lineStart,
	const VECTOR& LineEnd,

	const VECTOR& capsuleBottom,
	const VECTOR& capsuleTop,

	float capsuleRadius);