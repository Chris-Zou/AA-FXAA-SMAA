#include "ObjLight.h"

using namespace DirectX;

ObjLight::ObjLight(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	this->mDevice = _device;
	this->mDeviceContext = _deviceContext;
	CreateLightBuffer();
}

ObjLight::~ObjLight()
{
	SAFE_RELEASE(this->mLightBuffer);
}

HRESULT ObjLight::Render()
{
	HRESULT hr = S_OK;
	
	D3D11_MAPPED_SUBRESOURCE ms;
	if(FAILED(hr=this->mDeviceContext->Map(this->mLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
	{
		OutputDebugString(L"Failed to map OBJ lightbuffer.\n");
		return hr;
	}
	LightMat* dataptr;
	dataptr=(LightMat*)ms.pData;
	dataptr->light.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dataptr->light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dataptr->light.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dataptr->light.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	dataptr->light.Range = 120.0f;

	dataptr->material.Ambient = XMFLOAT4(0.90f, 0.90f, 0.90f, 1.0f);
	dataptr->material.Diffuse = XMFLOAT4(0.80f, 0.50f, 0.50f, 1.0f);
	dataptr->material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	dataptr->light.Position.x = 50;
	dataptr->light.Position.z = -50;
	dataptr->light.Position.y = 0;

	dataptr->eyeViewPos = XMFLOAT3(2, 0, 6);
	this->mDeviceContext->Unmap(this->mLightBuffer, 0);
	
	this->mDeviceContext->PSSetConstantBuffers(0, 1, &this->mLightBuffer);
	
	return hr;
}

HRESULT ObjLight::CreateLightBuffer()
{
	HRESULT hr=S_OK;
	
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(LightMat);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags=0;
	bd.StructureByteStride=0;
	
	if(FAILED(hr=this->mDevice->CreateBuffer(&bd, NULL, &this->mLightBuffer)))
	{
		OutputDebugString(L"Failed to create OBJ lightbuffer.\n");
		return hr;
	}

	return hr;
}
