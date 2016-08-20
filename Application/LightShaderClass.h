#ifndef LIGHTSHADERCLASS_H
#define LIGHTSHADERCLASS_H

#include "stdafx.h"
#include "Camera.h"
#include "Shader.h"
#include "Buffer.h"
#include <stdexcept>

using namespace DirectX;

__declspec(align(16))class LightShaderClass
{
	struct LightBuffer
	{
		DirectX::XMFLOAT4 DiffuseColor;
		DirectX::XMFLOAT3 LightDirection;
		float padding;
	};
	struct WVPBuffer
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};

private:
	Shader*					mVPShader;
	Buffer*					mMatrixBuffer;
	Buffer*					mLightBuffer;

public:
	LightShaderClass(ID3D11Device* device);
	~LightShaderClass();
	HRESULT Render(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, Camera* camera, int indexCount, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor);
	void* operator new(size_t i)
	{
		return _mm_malloc(i,16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
};

#endif