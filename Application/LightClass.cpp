#include "lightclass.h"


LightClass::LightClass()
{
}

LightClass::~LightClass()
{
}


void LightClass::SetAmbientColor(float _red, float _green, float _blue, float _alpha)
{
	this->mAmbientColor = XMFLOAT4( _red, _green, _blue, _alpha );
	return;
}


void LightClass::SetDiffuseColor(float _red, float _green, float _blue, float _alpha)
{
	this->mDiffuseColor = XMFLOAT4( _red, _green, _blue, _alpha );
	return;
}


void LightClass::SetDirection(float _x, float _y, float _z)
{
	this->mDirection = XMFLOAT3( _x, _y, _z );
	return;
}


XMFLOAT4 LightClass::GetAmbientColor()
{
	return this->mAmbientColor;
}


XMFLOAT4 LightClass::GetDiffuseColor()
{
	return this->mDiffuseColor;
}


XMFLOAT3 LightClass::GetDirection()
{
	return this->mDirection;
}