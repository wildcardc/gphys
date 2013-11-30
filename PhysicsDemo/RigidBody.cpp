#include "RigidBody.h"


RigidBody::RigidBody(XMVECTOR position, XMVECTOR size, float mass)
	:position(position),size(size),mass(mass)
{
}


RigidBody::~RigidBody(void)
{
}
