#include "FXAA.h"
#include <stdexcept>

FXAA::FXAA(ID3D11Device* device, DirectX::XMINT2 screenSize, ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV)
{
	this->backBufferRTV = RTV;
	this->backBufferDSV = DSV;

	//Shaders
	SHADER_DESCRIPTION sd;
	sd.vertexFilePath = "Assets/Shaders/FXAAVertexShader.hlsl";
	sd.vertexHeader = "VS_FXAA";
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

	sd.pixelFilePath = "Assets/Shaders/FXAAPixelShader.hlsl";
	//sd.pixelFilePath = "Assets/Shaders/FXAAPS.hlsl";
	sd.pixelHeader = "PS_FXAA";

	this->fxaaPixelShader = new Shader(device, sd);
	delete[]sd.inputDesc;

	//
	SHADER_DESCRIPTION sdl;
	sdl.vertexFilePath = "Assets/Shaders/FXAALumaVertexShader.hlsl";
	sdl.vertexHeader = "VS_FXAALUMA";
	sdl.vertexNumElem = 2;
	sdl.inputDesc = new D3D11_INPUT_ELEMENT_DESC[2];
	
	sdl.inputDesc[0].SemanticName = "POSITION";
	sdl.inputDesc[0].SemanticIndex = 0;
	sdl.inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	sdl.inputDesc[0].InputSlot = 0;
	sdl.inputDesc[0].AlignedByteOffset = 0;
	sdl.inputDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sdl.inputDesc[0].InstanceDataStepRate = 0;
	
	sdl.inputDesc[1].SemanticName = "TEXCOORD";
	sdl.inputDesc[1].SemanticIndex = 0;
	sdl.inputDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	sdl.inputDesc[1].InputSlot = 0;
	sdl.inputDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	sdl.inputDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sdl.inputDesc[1].InstanceDataStepRate = 0;
	
	sdl.pixelFilePath = "Assets/Shaders/FXAALumaPixelShader.hlsl";
	sdl.pixelHeader = "PS_FXAALUMA";

	this->fxaaLumaPixelShader = new Shader(device, sdl);
	delete[]sdl.inputDesc;

	CreateRenderTarget(device, screenSize);
	CreateSampler(device, screenSize);

	//Buffers
	BUFFER_DESCRIPTION fxaabd;
	fxaabd.Type = CONSTANT_BUFFER_VSPS;
	fxaabd.Usage = BUFFER_CPU_WRITE_DISCARD;
	fxaabd.TypeSize = sizeof(FXAABuffer);
	fxaabd.NumElem = 2;
	fxaabd.InitData = 0;
	this->fxaaBuffer = new Buffer(device, fxaabd);
}

FXAA::~FXAA()
{
	delete this->fxaaPixelShader;
	delete this->fxaaLumaPixelShader;
	delete this->fxaaBuffer;
}

void FXAA::Render(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* texture, DirectX::XMINT2 screenSize)
{
	float clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->ClearRenderTargetView(this->rTV, clear);

	//First render pass to set the luma
	{
		deviceContext->OMSetRenderTargets(1, &this->rTV, this->backBufferDSV);
		deviceContext->PSSetShaderResources(0, 1, &texture);
		this->fxaaLumaPixelShader->RenderIndexed(deviceContext, bilinearSampler, 6);
	}
	
	ID3D11ShaderResourceView* nullSrv = NULL;
	for (int i = 0; i<4; i++)
	{
		deviceContext->PSSetShaderResources(i, 1, &nullSrv);
	}

	//Second render pass to apply the FXAA
	//Update constant buffer

	HRESULT hr = S_OK;

	FXAABuffer* dataPtr = (FXAABuffer*)this->fxaaBuffer->Map(deviceContext);

	dataPtr->rcpFrame.x = 1.0f / (float)screenSize.x;
	dataPtr->rcpFrame.y = 1.0f / (float)screenSize.y;

	dataPtr->rcpFrameOpt.x = 2.0f / (float)screenSize.x;
	dataPtr->rcpFrameOpt.y = 2.0f / (float)screenSize.y;
	dataPtr->rcpFrameOpt.w = 0.5f / (float)screenSize.x;
	dataPtr->rcpFrameOpt.z = 0.5f / (float)screenSize.y;

	this->fxaaBuffer->Unmap(deviceContext);
	this->fxaaBuffer->ApplyBuffer(deviceContext, 0, 0);

	{
		deviceContext->OMSetRenderTargets(1, &this->backBufferRTV, this->backBufferDSV);
		deviceContext->PSSetShaderResources(0, 1, &this->sRV);
		this->fxaaPixelShader->RenderIndexed(deviceContext, bilinearSampler, 6);
	}
}

void FXAA::CreateRenderTarget(ID3D11Device* device, DirectX::XMINT2 screenSize)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = screenSize.x;
	textureDesc.Height = screenSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	HRESULT hr = S_OK;
	hr = device->CreateTexture2D(&textureDesc, NULL, &this->rTT);
	if (FAILED(hr))
		throw std::runtime_error("create RenderTargetTexture Texture2D");
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	hr = device->CreateRenderTargetView(this->rTT, &renderTargetViewDesc, &this->rTV);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View");
	shaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	hr = device->CreateShaderResourceView(this->rTT, &shaderResourceViewDesc, &this->sRV);
	if (FAILED(hr))
		throw std::runtime_error("create sRV");
}

void FXAA::CreateSampler(ID3D11Device* device, DirectX::XMINT2 screenSiz)
{
	//Create SamplerState
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = S_OK;
	hr = device->CreateSamplerState(&samplerDesc, &this->bilinearSampler);
	if (FAILED(hr))
		throw std::runtime_error("Create SamplerState - mSamplerState");
}