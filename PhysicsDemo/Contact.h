#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Contact
{
public:
	Contact(void);
	~Contact(void);

	XMVECTOR position;
	XMVECTOR normal;
	float depth;
	int body1,body2;
};

