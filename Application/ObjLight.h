#ifndef OBJLIGHT_H
#define OBJLIGHT_H

#include <d3d11.h>
#include <DirectXMath.h>
#include "stdafx.h"

class ObjLight
{
private:
	struct Light
	{
		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular;
		DirectX::XMFLOAT3 Position;
		float Range;
		DirectX::XMFLOAT3 Att;
		float pad;
	};
	struct Material
	{
		Material()
		{
			ZeroMemory(this, sizeof(this));
		}
		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular;
		DirectX::XMFLOAT4 Reflect;
	};
	struct LightMat
	{
		Light				light;
		Material			material;
		DirectX::XMFLOAT3	eyeViewPos;
		float				pad;
	};

private:
	ID3D11Device*				mDevice;
	ID3D11DeviceContext*		mDeviceContext;
	ID3D11Buffer*				mLightBuffer;
	LightMat					mPointLight;

public:
	ObjLight(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~ObjLight();
	HRESULT CreateLightBuffer();
	HRESULT Render();

};

#endif