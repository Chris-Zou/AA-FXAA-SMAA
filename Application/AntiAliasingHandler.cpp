#include "AntiAliasingHandler.h"

AntiAliasingHandler::AntiAliasingHandler(ID3D11Device* device, DirectXClass* directx, DirectX::XMINT2 screenSize, ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV)
{
	this->currentAA = 0;
	this->prevAA = -1;
	this->directX = directx;

	CreatePassThroughShader(device);
	this->fxaa = new FXAA(device, screenSize, RTV, DSV);
	this->smaa = new SMAA(device, screenSize, RTV, DSV);
}

AntiAliasingHandler::~AntiAliasingHandler()
{
	delete this->passThrough;
	delete this->fxaa;
	delete this->smaa;
}

void AntiAliasingHandler::Render(int method, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, int indexCount, ID3D11SamplerState* samplerState, DirectX::XMINT2 screenSize)
{
	switch (method)
	{
	case AntiAliasingMethod::NOAA:
	{
		this->directX->SetBackBufferAsRenderTarget();
		this->directX->ClearBackBuffer(DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
		deviceContext->PSSetShaderResources(0, 1, &texture);
		this->passThrough->RenderIndexed(deviceContext, samplerState, indexCount);
		break;
	}
	case AntiAliasingMethod::FXAA:
	{
		this->fxaa->Render(deviceContext, samplerState, texture, screenSize);
		break;
	}
	case AntiAliasingMethod::SMAA:
	{
		this->smaa->Render(deviceContext, samplerState, texture, screenSize);
		break;
	}
	}
}

void AntiAliasingHandler::CreatePassThroughShader(ID3D11Device * device)
{
	//Shaders
	SHADER_DESCRIPTION sd;
	sd.vertexFilePath = "Assets/Shaders/PassThroughVS.hlsl";
	sd.vertexHeader = "VS_PassThrough";
	sd.vertexNumElem = 2;
	sd.inputDesc = new D3D11_INPUT_ELEMENT_DESC[2];

	sd.inputDesc[0].SemanticName = "POSITION";
	sd.inputDesc[0].SemanticIndex = 0;
	sd.inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	sd.inputDesc[0].InputSlot = 0;
	sd.inputDesc[0].AlignedByteOffset = 0;
	sd.inputDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[0].InstanceDataStepRate = 0;

	sd.inputDesc[1].SemanticName = "TEXCOORD";
	sd.inputDesc[1].SemanticIndex = 0;
	sd.inputDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	sd.inputDesc[1].InputSlot = 0;
	sd.inputDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	sd.inputDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[1].InstanceDataStepRate = 0;

	sd.pixelFilePath = "Assets/Shaders/PassThroughPS.hlsl";
	sd.pixelHeader = "PS_PassThrough";

	this->passThrough = new Shader(device, sd);
	delete[]sd.inputDesc;
}
