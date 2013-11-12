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

		static void TW_CALL GetCurrentLengthCallback(void *value, void *clientData)
		{
			*static_cast<float *>(value) = static_cast<Spring*>(clientData)->currentLength();
		} 
};

