#include "ColObj.h"


ColObj::ColObj(XMVECTOR pos)
	: pos(pos)
{
	v = f = XMVectorZero();
}


ColObj::~ColObj(void)
{
}
