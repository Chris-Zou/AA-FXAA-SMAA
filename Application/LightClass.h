#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include <DirectXMath.h>
using namespace DirectX;

class LightClass
{
private:
	XMFLOAT4 mAmbientColor;
	XMFLOAT4 mDiffuseColor;
	XMFLOAT3 mDirection;

public:
	LightClass();
	~LightClass();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetDirection(float, float, float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetDirection();
};

#endif