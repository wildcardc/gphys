#pragma once

#include "RigidBody.h"

#include <DirectXMath.h>
using namespace DirectX;

class Contact
{
public:
	Contact(void);
	~Contact(void);

	XMVECTOR position;
	RigidBody *a, *b;
	XMVECTOR normal;
	float depth;
};

