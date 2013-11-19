#pragma once

#include <DirectXMath.h>
using namespace DirectX;
//#include <DXUT.h>

class MassPoint
{
public:
	MassPoint(void);
	~MassPoint(void);

	MassPoint(XMVECTOR position, float mass, float damping);

	public:
		XMVECTOR position;
		XMVECTOR velocity;
		XMVECTOR force;
		float mass;
		float damping;
};

