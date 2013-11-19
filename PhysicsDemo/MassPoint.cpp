#include "MassPoint.h"


MassPoint::MassPoint(void)
{
}


MassPoint::~MassPoint(void)
{
}

MassPoint::MassPoint(XMVECTOR position, float mass, float damping)
	:position(position), mass(mass), damping(damping)
{
	velocity = XMVectorZero();

}