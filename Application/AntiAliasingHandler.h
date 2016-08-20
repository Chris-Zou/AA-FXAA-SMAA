#pragma once

#include <d3d11.h>
#include "Shader.h"
#include "FXAA.h"
#include "SMAA.h"
#include "Buffer.h"
#include <DirectXMath.h>
#include "DirectXClass.h"

class AntiAliasingHandler
{
private:
	Shader*				passThrough;
	FXAA*				fxaa;
	SMAA*				smaa;
	int					currentAA;
	int					prevAA;
	DirectXClass*		directX;


private:
	void CreatePassThroughShader(ID3D11Device* device);

public:
	AntiAliasingHandler(ID3D11Device* device, DirectXClass* directx, DirectX::XMINT2 screenSize, ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV);
	~AntiAliasingHandler();
	void Render(int method, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, int indexCount, ID3D11SamplerState* samplerState, DirectX::XMINT2 screenSize);
};

namespace AntiAliasingMethod
{
	const int NOAA = 0;
	const int FXAA = 1;
	const int SMAA = 2;
};