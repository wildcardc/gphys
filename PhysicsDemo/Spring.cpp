#include "Spring.h"


Spring::Spring(MassPoint* p1, MassPoint* p2, float k)
	: point1(p1), point2(p2), stiffness(k), initialLength(currentLength())
{
	
}


Spring::~Spring(void)
{
}

float Spring::currentLength()
{
	// i know how to count to potato
	return abs(XMVectorGetX(XMVector3Length(point1->position - point2->position)));
}