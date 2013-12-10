#pragma once

#include <DXUT.h>

#include "IPhysicSystem.h"
#include "RigidBody.h"



class RigidBodySystem : private IPhysicSystem
{
public:
	RigidBodySystem(void);
	~RigidBodySystem(void);

	std::vector<RigidBody*> rigidBodies;

	void IPhysicSystem::DoPhysics(double dt);
	void IPhysicSystem::DrawPhysicSystem(ID3D11DeviceContext* d3D11DeviceContext);

};

