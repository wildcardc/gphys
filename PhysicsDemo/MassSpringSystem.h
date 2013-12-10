#pragma once

#include <random>
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

	vector<MassPoint*> points;
	vector<Spring*> springs;

	typedef enum
	 {
		INTEGRATOR_EXPLICIT_EULER = 0,
		INTEGRATOR_MIDPOINT,
		INTEGRATOR_LEAPFROG,
	
		INTEGRATOR_LAST,
	 } Integrators;

	Integrators g_Integrator;// = INTEGRATOR_LEAPFROG;
 
	TwEnumVal* twIntegratorEV;// = { {INTEGRATOR_EXPLICIT_EULER, "Explicit Euler"}, {INTEGRATOR_MIDPOINT, "Midpoint"}, {INTEGRATOR_LEAPFROG, "Leapfrog"} };

private:
	
	void ClearForces();
	MassSpringSystem* Euler(float dt, MassSpringSystem* mss);
	MassSpringSystem* MidPoint(float dt, MassSpringSystem* mss);
	MassSpringSystem* LeapFrog(float dt, MassSpringSystem* mss);

};