#pragma once

#include <DXUT.h>
#include <DirectXMath.h>
using namespace DirectX;

class IPhysicSystem
{
public:
	
	virtual void DoPhysics(double dt) = 0;

	virtual void DrawPhysicSystem(ID3D11DeviceContext* pd3dImmediateContext) = 0;

protected:

	XMVECTOR g_G;

};