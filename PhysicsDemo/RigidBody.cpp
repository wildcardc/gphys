#include "RigidBody.h"

RigidBody::RigidBody(XMVECTOR position, XMVECTOR size, float mass)
	:position(position),size(size),mass(mass)
{
	//Masspoints
	masspoints[0] = XMVectorSet(XMVectorGetX(size/2),-XMVectorGetY(size/2),-XMVectorGetZ(size/2),1);
	masspoints[1] = XMVectorSet(XMVectorGetX(size/2),-XMVectorGetY(size/2),XMVectorGetZ(size/2),1);
	masspoints[2] = XMVectorSet(XMVectorGetX(size/2),XMVectorGetY(size/2),-XMVectorGetZ(size/2),1);
	masspoints[3] = XMVectorSet(XMVectorGetX(size/2),XMVectorGetY(size/2),XMVectorGetZ(size/2),1);
	masspoints[4] = XMVectorSet(-XMVectorGetX(size/2),-XMVectorGetY(size/2),-XMVectorGetZ(size/2),1);
	masspoints[5] = XMVectorSet(-XMVectorGetX(size/2),-XMVectorGetY(size/2),XMVectorGetZ(size/2),1);
	masspoints[6] = XMVectorSet(-XMVectorGetX(size/2),XMVectorGetY(size/2),-XMVectorGetZ(size/2),1);
	masspoints[7] = XMVectorSet(-XMVectorGetX(size/2),XMVectorGetY(size/2),XMVectorGetZ(size/2),1);

	//planes
	planes[0][0] = 0; planes[0][1] = 1; planes[0][2] = 2; planes[0][3] = 3;
	planes[1][0] = 0; planes[1][1] = 1; planes[1][2] = 4; planes[1][3] = 5;
	planes[2][0] = 4; planes[2][1] = 5; planes[2][2] = 6; planes[2][3] = 7;
	planes[3][0] = 2; planes[3][1] = 3; planes[3][2] = 6, planes[3][3] = 7;
	planes[4][0] = 1; planes[4][1] = 3; planes[4][2] = 5; planes[4][3] = 7;
	planes[5][0] = 0; planes[5][1] = 2; planes[5][2] = 4; planes[5][3] = 6;


	//Initial inertia tensor
	i0Inverted = XMMatrixSet(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	for(int i  = 0; i < 8; i++)
	{
		i0Inverted += XMMatrixSet(
			XMVectorGetX(masspoints[i]) * XMVectorGetX(masspoints[i]),	XMVectorGetX(masspoints[i]) * XMVectorGetY(masspoints[i]),	XMVectorGetX(masspoints[i]) * XMVectorGetZ(masspoints[i]),	0,
			XMVectorGetY(masspoints[i]) * XMVectorGetX(masspoints[i]),	XMVectorGetY(masspoints[i]) * XMVectorGetY(masspoints[i]),	XMVectorGetY(masspoints[i]) * XMVectorGetZ(masspoints[i]),	0,
			XMVectorGetZ(masspoints[i]) * XMVectorGetX(masspoints[i]),	XMVectorGetZ(masspoints[i]) * XMVectorGetY(masspoints[i]),	XMVectorGetZ(masspoints[i]) * XMVectorGetZ(masspoints[i]),	0,
			0,															0,															0,															0);
	}
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

XMVECTOR RigidBody::masspoint_world(int i)
{
	return XMVector3Transform(this->masspoints[i], XMMatrixRotationQuaternion(this->orientation)) + this->position;
}

RigidBody::~RigidBody(void)
{
}
