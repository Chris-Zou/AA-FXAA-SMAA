#pragma once

#include "DirectXClass.h"
#include "Buffer.h"
#include "Shader.h"
#include "Camera.h"
#include <DirectXMath.h>
#include "LightShaderClass.h"
#include "LightClass.h"
#include "OrthoWindowClass.h"
#include "AntiAliasingHandler.h"

class RenderModule
{
private:
	struct WVPBuffer
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};

private:
	ID3D11Device*		_device;
	ID3D11DeviceContext* _deviceContext;
	Buffer*	_matrixBuffer;
	Shader*	_vpShader;
	//Light
	LightShaderClass*	_lightShader;
	LightClass			_light;
	DirectXClass*		_directX;
	ID3D11SamplerState* _samplerState;
	OrthoWindowClass*	_orthoWindow;
	AntiAliasingHandler* _AAHandler;

private:
	HRESULT CreateShaders(ID3D11Device* device);

public:
	RenderModule(DirectXClass* directX, DirectX::XMINT2 screenSize);
	~RenderModule();

	void Update(Camera* camera);
	void RenderVertex(Camera* camera, ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState, DirectX::XMMATRIX* localspace, int numVertex);
	void RenderIndexed(Camera* camera, ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState, DirectX::XMMATRIX* localspace, int numIndex);
	void RenderLight(Camera* camera, int aliasingMethod);
	void RenderAntiAliasing(int method);
};
