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
	float mass;

};

