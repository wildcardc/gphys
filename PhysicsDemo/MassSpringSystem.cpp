#include "MassSpringSystem.h"


MassSpringSystem::MassSpringSystem(void)
{
}


MassSpringSystem::~MassSpringSystem(void)
{
	for(auto i = points.rbegin(); i != points.rend(); i++)
		delete *i;
		
	for(auto i = springs.rbegin(); i != springs.rend(); i++)
		delete *i;
}
