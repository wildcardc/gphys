#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class ColObj
{
public:
	ColObj(XMVECTOR);
	~ColObj(void);

	XMVECTOR pos, pos2;
	XMVECTOR v, f;
};

