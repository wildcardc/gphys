#include "Spring.h"
#include "MassSpringSystem.h"

Spring::Spring(int p1, int p2, float k, MassSpringSystem* mss)
	: point1(p1), point2(p2), stiffness(k), initialLength(currentLength(mss))
{
	
}


Spring::~Spring(void)
{
}

float Spring::currentLength(MassSpringSystem* mss)
{
	// i know how to count to potato
	return abs(XMVectorGetX(XMVector3Length(mss->points[point1]->position - mss->points[point2]->position)));
}

/*void TW_CALL Spring::GetCurrentLengthCallback(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<Spring*>(clientData)->currentLength((MassSpringSystem*) clientData);
} */