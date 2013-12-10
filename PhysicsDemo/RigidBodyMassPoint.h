#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class RigidBodyMassPoint
{
public:
	RigidBodyMassPoint(XMVECTOR localPosition, XMVECTOR worldPosition, float mass);
	~RigidBodyMassPoint(void);

	XMVECTOR localPosition;
	XMVECTOR worldPosition;
	XMVECTOR velocity;
	XMVECTOR force;
	float mass;
};

