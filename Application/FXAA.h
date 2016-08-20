#pragma once

#include "Shader.h"
#include "Buffer.h"
#include <DirectXMath.h>

class FXAA
{
private:
	struct FXAABuffer
	{
		DirectX::XMFLOAT2 rcpFrame;
		DirectX::XMFLOAT4 rcpFrameOpt;
	};
private:
	Shader* fxaaPixelShader;
	Shader* fxaaLumaPixelShader;
	//Buffers
	Buffer* fxaaBuffer;

	ID3D11RenderTargetView* backBufferRTV;
	ID3D11DepthStencilView* backBufferDSV;

	void CreateRenderTarget(ID3D11Device* device, DirectX::XMINT2 screenSize);
	void CreateSampler(ID3D11Device* device, DirectX::XMINT2 screenSize);
	ID3D11Texture2D*			rTT;
	ID3D11RenderTargetView*		rTV;
	ID3D11ShaderResourceView*	sRV;
	ID3D11SamplerState*			bilinearSampler;

public:
	FXAA(ID3D11Device* device, DirectX::XMINT2 screenSize, ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV);
	~FXAA();
	void Render(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* backbuffer, DirectX::XMINT2 screenSize);
};