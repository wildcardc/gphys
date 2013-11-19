#pragma once

#include "MassPoint.h"
#include <DirectXMath.h>
#include <AntTweakBar.h>

class MassSpringSystem;

class Spring
{
public:
	Spring(int p1, int p2, float k, MassSpringSystem* mss);
	~Spring(void);

	public:
		int point1;
		int point2;
		float stiffness;
		const float initialLength;
		float currentLength(MassSpringSystem* mss);

	//static void TW_CALL GetCurrentLengthCallback(void *value, void *clientData);	
};

