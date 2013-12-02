#pragma once

#include <DirectXMath.h>

#include "IPhysicSystem.h"

using namespace DirectX;

class RigidBody : private IPhysicSystem
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

