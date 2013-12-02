#pragma once

#include <DXUT.h>
#include <vector>
using std::vector;

#include "Spring.h"
#include "MassPoint.h"
#include "IPhysicSystem.h"

class MassSpringSystem : public IPhysicSystem
{
public:
	MassSpringSystem(void);
	~MassSpringSystem(void);

	MassSpringSystem& operator=( const MassSpringSystem& rhs );
	MassSpringSystem( const MassSpringSystem& other );

	void IPhysicSystem::DoPhysics(double dt);
	void IPhysicSystem::DrawPhysicSystem(ID3D11DeviceContext* iD3D11DeviceContext);


public:
	vector<MassPoint*> points;
	vector<Spring*> springs;
};