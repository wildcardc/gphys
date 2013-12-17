#include "RigidBodyMassPoint.h"


RigidBodyMassPoint::RigidBodyMassPoint(XMVECTOR localPosition, XMVECTOR worldPosition, float mass)
	:localPosition(localPosition),worldPosition(worldPosition),mass(mass)
{
	velocity = XMVectorZero();
	force = XMVectorZero();
}

RigidBodyMassPoint::RigidBodyMassPoint()
{
}

RigidBodyMassPoint::~RigidBodyMassPoint(void)
{
}
