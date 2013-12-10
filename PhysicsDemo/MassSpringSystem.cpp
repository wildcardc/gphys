#include "MassSpringSystem.h"


MassSpringSystem::MassSpringSystem(void)
{
	g_G = XMVectorSet(.0f, -9.81f * .05f, .0f, .0f);

	g_Integrator = INTEGRATOR_LEAPFROG;
 
	TwEnumVal twEnumVal[] = { {INTEGRATOR_EXPLICIT_EULER, "Explicit Euler"}, {INTEGRATOR_MIDPOINT, "Midpoint"}, {INTEGRATOR_LEAPFROG, "Leapfrog"} };
	twIntegratorEV = twEnumVal;

}


MassSpringSystem::~MassSpringSystem(void)
{
	for(auto i = points.rbegin(); i != points.rend(); i++)
		delete *i;
		
	for(auto i = springs.rbegin(); i != springs.rend(); i++)
		delete *i;
}


MassSpringSystem& MassSpringSystem::operator=( const MassSpringSystem& rhs )
{
	for(auto i = rhs.points.cbegin(); i != rhs.points.cend(); i++)
		points.push_back(new MassPoint(**i));

	for(auto i = rhs.springs.cbegin(); i != rhs.springs.cend(); i++)
		springs.push_back(new Spring(**i));

	return *this;
}

MassSpringSystem::MassSpringSystem( const MassSpringSystem& other )
{
	for(auto i = other.points.cbegin(); i != other.points.cend(); i++)
		points.push_back(new MassPoint(**i));

	for(auto i = other.springs.cbegin(); i != other.springs.cend(); i++)
		springs.push_back(new Spring(**i));
}

void MassSpringSystem::DoPhysics(double dt)
{
	//clear forces
	ClearForces();

	this = IntegratorFuncs[g_Integrator](dt, this);

	// collisions
	for (auto i = g_MassSpringSystem->points.begin(); i != g_MassSpringSystem->points.end(); i++)
    {
		// boundary check -y; has to be done first for now
		if(XMVectorGetY((*i)->position) < -.5f)
			(*i)->position = XMVectorSetY((*i)->position, -.5f);
		
		// really really crude MP-MP collision
		for (auto j = i + 1; j != g_MassSpringSystem->points.end(); )
		{
			assert(*i != *j);

			auto d = (*i)->position - (*j)->position;
			float l = XMVectorGetX(XMVector3Length(d));

			if (l < g_fSphereSize)
			{
				// collision response away from Y plane
				auto top = XMVectorGetY((*j)->position) < XMVectorGetY((*i)->position) ? *i : *j;
				auto n = XMVector3Normalize(d);
				auto respone = n * (g_fSphereSize - l);

				if(XMVectorGetY(n) > 0)
					top->position += respone;
				else
					top->position -= respone;
			}
		}
	}
}

void MassSpringSystem::ClearForces()
{
	for(auto i = points.begin(); i != points.end(); i++)
	{
		(*i)->force = XMVectorZero();
	}
}

void ApplyForces(MassSpringSystem* mss)
{

	int k = 0;
	//Update Springs
	for (auto i = mss->springs.begin(); i != mss->springs.end(); i++)
	{
		//calculate spring forces
		float l = (*i)->currentLength(mss);
		if(l == 0)
			l = .001f;

		XMVECTOR f = -(*i)->stiffness*(l - (*i)->initialLength) * (mss->points[(*i)->point1]->position - mss->points[(*i)->point2]->position) / l;

		//overwrite old forces with spring forces
		mss->points[(*i)->point1]->force += f;
		mss->points[(*i)->point2]->force += -f;
		k++;
	}

	for (auto i = mss->points.begin(); i != mss->points.end(); i++)
    {
		//update forces with gravitation forces
		(*i)->force -= (*i)->damping * (*i)->velocity;
		//(*i)->force -= (*i)->damping * (*i)->force;
	}

	//old version -----------------------------------------------------------------
	//int k = 0;
	////Update Springs
	//for (auto i = mss->springs.begin(); i != mss->springs.end(); i++)
	//{
	//	//calculate spring forces
	//	float l = (*i)->currentLength(mss);
	//	if(l == 0)
	//		l = .001f;

	//	XMVECTOR f = -(*i)->stiffness*(l - (*i)->initialLength) * (mss->points[(*i)->point1]->position - mss->points[(*i)->point2]->position) / l;

	//	//overwrite old forces with spring forces
	//	mss->points[(*i)->point1]->force = f;
	//	mss->points[(*i)->point2]->force = -f;
	//	k++;
	//}

	//for (auto i = mss->points.begin(); i != mss->points.end(); i++)
 //   {
	//	//update forces with gravitation forces
	//	//(*i)->force += g_G * (*i)->mass - (*i)->damping * (*i)->velocity;
	//	(*i)->force -= (*i)->damping * (*i)->velocity;
	//}
	
	// external forces..
}

MassSpringSystem* MassSpringSystem::Euler(float dt, MassSpringSystem* mss)
{
	MassSpringSystem* out_mss = new MassSpringSystem(*mss);

	ApplyForces(out_mss);

	for (auto i = out_mss->points.begin(); i != out_mss->points.end(); i++)
    {
		//update positions
		(*i)->position += dt * (*i)->velocity;

		//update velocity
		//m*a = Fint + Fext - y*v
		//F = m * a = kg * m/s^2 - m/s * kg/s
		//v = (Fint/m + Fext/m - yv) * dt
		(*i)->velocity += ((*i)->force / (*i)->mass + g_G) * dt;
	}

	return out_mss;
}

// invalid value to always trigger the initial leapfrog step
Integrators g_PrevInt = INTEGRATOR_LAST;

MassSpringSystem* MassSpringSystem::LeapFrog(float dt, MassSpringSystem* mss)
{
	ApplyForces(mss);

	for (auto i = mss->points.begin(); i != mss->points.end(); i++)
	{
		// while this doesnt really seem to do much, we'll just keep it for now
		if(g_PrevInt != INTEGRATOR_LEAPFROG)
			(*i)->velocity += ((*i)->force / (*i)->mass + g_G) * dt/2;
		else
			(*i)->velocity += ((*i)->force / (*i)->mass + g_G) * dt;
			
		//update positions
		(*i)->position += dt * (*i)->velocity;
	}
	
	return mss;
}

MassSpringSystem* MassSpringSystem::MidPoint(float dt, MassSpringSystem* mss)
{
	MassSpringSystem* euler = Euler(dt/2, mss);

	ApplyForces(euler);

	for (int i = 0; i < mss->points.size(); i++)
    {
		//update positions
		mss->points[i]->position += dt * euler->points[i]->velocity;

		//update velocity
		mss->points[i]->velocity = euler->points[i]->velocity;
		//mss->points[i]->velocity += euler->points[i]->force / euler->points[i]->mass * dt + g_G * dt;
	}

	delete euler;

	return mss;
}


void MassSpringSystem::DrawPhysicSystem(ID3D11DeviceContext* d3D11DeviceContext)
{
    ;
}