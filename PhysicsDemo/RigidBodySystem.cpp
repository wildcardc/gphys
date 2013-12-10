#include "RigidBodySystem.h"


RigidBodySystem::RigidBodySystem(void)
{
	g_G = XMVectorSet(.0f, -9.81f * .05f, .0f, .0f);
}


RigidBodySystem::~RigidBodySystem(void)
{
}

void RigidBodySystem::DoPhysics(double dt)
{
	;
}

void RigidBodySystem::DrawPhysicSystem(ID3D11DeviceContext* d3D11DeviceContext)
{
	;
}
