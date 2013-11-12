#pragma once

#include <vector>
using std::vector;

#include "Spring.h"
#include "MassPoint.h"

class MassSpringSystem
{
public:
	MassSpringSystem(void);
	~MassSpringSystem(void);

public:
	vector<MassPoint*> points;
	vector<Spring*> springs;
};

