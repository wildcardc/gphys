#include "RigidBody.h"


RigidBody::RigidBody(XMVECTOR position, XMVECTOR size, float mass)
	:position(position),size(size),mass(mass)
{
	//Masspoints
	masspoints[0].localPosition = XMVectorSet(XMVectorGetX(size),-XMVectorGetY(size),-XMVectorGetZ(size),1);
	masspoints[1].localPosition = XMVectorSet(XMVectorGetX(size),-XMVectorGetY(size),XMVectorGetZ(size),1);
	masspoints[2].localPosition = XMVectorSet(XMVectorGetX(size),XMVectorGetY(size),-XMVectorGetZ(size),1);
	masspoints[3].localPosition = XMVectorSet(XMVectorGetX(size),XMVectorGetY(size),XMVectorGetZ(size),1);
	masspoints[4].localPosition = XMVectorSet(-XMVectorGetX(size),-XMVectorGetY(size),-XMVectorGetZ(size),1);
	masspoints[5].localPosition = XMVectorSet(-XMVectorGetX(size),-XMVectorGetY(size),XMVectorGetZ(size),1);
	masspoints[6].localPosition = XMVectorSet(-XMVectorGetX(size),XMVectorGetY(size),-XMVectorGetZ(size),1);
	masspoints[7].localPosition = XMVectorSet(-XMVectorGetX(size),XMVectorGetY(size),XMVectorGetZ(size),1);

	for(int i  = 0; i < 8; i++)
		masspoints[i].worldPosition = masspoints[i].localPosition + position;

	//Initial inertia tensor
	i0Inverted = XMMatrixSet(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	/*for(int i  = 0; i < 8; i++)
	{
		i0Inverted += XMMatrixSet(
			XMVectorGetX(masspoints[i]) * XMVectorGetX(masspoints[i]),	XMVectorGetX(masspoints[i]) * XMVectorGetY(masspoints[i]),	XMVectorGetX(masspoints[i]) * XMVectorGetZ(masspoints[i]),	0,
			XMVectorGetY(masspoints[i]) * XMVectorGetX(masspoints[i]),	XMVectorGetY(masspoints[i]) * XMVectorGetY(masspoints[i]),	XMVectorGetY(masspoints[i]) * XMVectorGetZ(masspoints[i]),	0,
			XMVectorGetZ(masspoints[i]) * XMVectorGetX(masspoints[i]),	XMVectorGetZ(masspoints[i]) * XMVectorGetY(masspoints[i]),	XMVectorGetZ(masspoints[i]) * XMVectorGetZ(masspoints[i]),	0,
			0,															0,															0,															0);
	}*/
	i0Inverted *= mass/8.0f;
	float trace = XMVectorGetX(i0Inverted.r[0]) + XMVectorGetY(i0Inverted.r[1]) + XMVectorGetZ(i0Inverted.r[2]);
	i0Inverted = XMMatrixIdentity() * trace - i0Inverted;
	i0Inverted = XMMatrixInverse(0,i0Inverted);
	i0Inverted.r[3] = XMVectorSetW(i0Inverted.r[3],1);
	
	//Velocity
	velocity = XMVectorZero();
	

	//Orientation
	orientation = XMQuaternionIdentity();


	//angular momentum
	angularMomentum = XMVectorZero();


	//current inertia tensor
	iInverted = XMMatrixRotationQuaternion(orientation) * i0Inverted * XMMatrixTranspose(XMMatrixRotationQuaternion(orientation));
	

	//angular velocity
	angularVelocity = XMVectorZero();

}


RigidBody::~RigidBody(void)
{
}
