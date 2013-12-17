#pragma once

#include <DirectXMath.h>

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
	XMVECTOR masspoints[8];
	int planes[6][4];
	XMMATRIX i0Inverted;
	XMMATRIX iInverted;
	XMVECTOR angularMomentum;
	XMVECTOR angularVelocity;
	float mass;

};

