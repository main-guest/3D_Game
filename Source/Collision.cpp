#include <algorithm>
#include "Collision.h"

static float Clamp(float value, float minValue, float maxValue)
{
	if (value < minValue)return minValue;
	if (value > maxValue)return maxValue;

	return value;
}

static float GetSegmentDistanceSq(
	const VECTOR& p1,
	const VECTOR& q1,
	const VECTOR& p2,
	const VECTOR& q2)
{
	VECTOR d1 = VSub(q1, p1);
	VECTOR d2 = VSub(q2, p2);
	VECTOR r = VSub(p1, p2);

	float a = VDot(d1, d1);
	float e = VDot(d2, d2);
	float f = VDot(d2, r);

	float s, t;

	if (a <= 0.0001f && e <= 0.0001f)
	{
		return VDot(r, r);
	}

	if (a <= 0.0001f)
	{
		s = 0.0f;
		t = Clamp(f / e, 0.0f, 1.0f);
	}
	else
	{
		float c = VDot(d1, r);

		if (e <= 0.0001f)
		{
			t = 0.0f;
			s = Clamp(-c / a, 0.0f, 1.0f);
		}
		else
		{
			float b = VDot(d1, d2);
			float denom = a * e - b * b;

			if (fabsf(denom) > 0.0001f)
			{
				s = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
			}
			else
			{
				s = 0.0f;
			}

			t = (b * s + f) / e;

			if (t < 0.0f)
			{
				t = 0.0f;
				s = Clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = Clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}

	VECTOR c1 = VAdd(p1, VScale(d1, s));
	VECTOR c2 = VAdd(p2, VScale(d2, t));

	VECTOR diff = VSub(c1, c2);

	return VDot(diff, diff);
}

bool LineCapsuleHit(
	const VECTOR& lineStart,
	const VECTOR& lineEnd,

	const VECTOR& capsuleBottom,
	const VECTOR& capsuleTop,

	float capsuleRadius)
{
	float distSq =
		GetSegmentDistanceSq(
			lineStart,
			lineEnd,

			capsuleBottom,
			capsuleTop);

	return distSq <= capsuleRadius * capsuleRadius;
}