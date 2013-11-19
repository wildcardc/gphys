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
