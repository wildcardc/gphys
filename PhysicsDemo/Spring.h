#pragma once

#include "MassPoint.h"
#include <DirectXMath.h>

class Spring
{
public:
	Spring(MassPoint* p1, MassPoint* p2, float k);
	~Spring(void);

	public:
		MassPoint* point1;
		MassPoint* point2;
		float stiffness;
		const float initialLength;
		float currentLength();
};

