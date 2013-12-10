#pragma once

#include <DirectXMath.h>
#include "RigidBodyMassPoint.h"

using namespace DirectX;

class RigidBody
{
public:
	RigidBody(XMVECTOR position, XMVECTOR size, float mass);
	~RigidBody(void);
	XMVECTOR orientation;
	XMVECTOR position;
	XMVECTOR size;
	XMVECTOR velocity;
	XMVECTOR forces;
	RigidBodyMassPoint masspoints[8];
	XMMATRIX i0Inverted;
	XMMATRIX iInverted;
	XMVECTOR angularMomentum;
	XMVECTOR angularVelocity;
	float mass;

};

