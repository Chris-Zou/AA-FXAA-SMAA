#include "SMAA.h"
#include <stdexcept>

SMAA::SMAA(ID3D11Device* device, DirectX::XMINT2 screenSize, ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV)
{
	this->backBufferRTV = RTV;
	this->backBufferDSV = DSV;

	this->areaTexSRV = CreateTextureFromRAWMemory(device, AREATEX_WIDTH, AREATEX_HEIGHT, AREATEX_PITCH, AREATEX_SIZE, DXGI_FORMAT_R8G8_UNORM, areaTexBytes);
	this->searchTexSRV = CreateTextureFromRAWMemory(device, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, SEARCHTEX_PITCH, SEARCHTEX_SIZE, DXGI_FORMAT_R8_UNORM, searchTexBytes);

	//Sampler
	CreateSampler(device);

	//Shaders
	//Edge detection
	{
		SHADER_DESCRIPTION sd;
		sd.vertexFilePath = "Assets/Shaders/SMAAEdgeDetectionVS.hlsl";
		sd.vertexHeader = "VS_SMAAEdgeDetection";
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

		sd.pixelFilePath = "Assets/Shaders/SMAAEdgeDetectionPS.hlsl";
		sd.pixelHeader = "PS_SMAAEdgeDetection";

		this->edgeDetectionShaders = new Shader(device, sd);
		delete[]sd.inputDesc;
	}

	//Blending weight calculations
	{
		SHADER_DESCRIPTION sd;
		sd.vertexFilePath = "Assets/Shaders/SMAABlendingWeightVS.hlsl";
		sd.vertexHeader = "VS_SMAABlendingWeight";
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

		sd.pixelFilePath = "Assets/Shaders/SMAABlendingWeightPS.hlsl";
		sd.pixelHeader = "PS_SMAABlendingWeight";

		this->blendingWeightShaders = new Shader(device, sd);
		delete[]sd.inputDesc;
	}

	//Neighbourhood Blending calculations
	{
		SHADER_DESCRIPTION sd;
		sd.vertexFilePath = "Assets/Shaders/SMAANeighbourhoodBlendingVS.hlsl";
		sd.vertexHeader = "VS_SMAANeighbourhoodBlending";
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

		sd.pixelFilePath = "Assets/Shaders/SMAANeighbourhoodBlendingPS.hlsl";
		sd.pixelHeader = "PS_SMAANeighbourhoodBlending";

		this->neighbourhoodBlendingShaders = new Shader(device, sd);
		delete[]sd.inputDesc;
	}

	//Buffers
	BUFFER_DESCRIPTION smaabd;
	smaabd.Type = CONSTANT_BUFFER_VSPS;
	smaabd.Usage = BUFFER_CPU_WRITE_DISCARD;
	smaabd.TypeSize = sizeof(SMAABuffer);
	smaabd.NumElem = 1;
	smaabd.InitData = 0;
	this->smaaBuffer = new Buffer(device, smaabd);

	//RenderTargets
	CreateRenderTargets(device, screenSize);
}

SMAA::~SMAA()
{
	delete this->edgeDetectionShaders;
	delete this->blendingWeightShaders;
	delete this->neighbourhoodBlendingShaders;
	delete this->smaaBuffer;
}

void SMAA::Render(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* backbuffer, DirectX::XMINT2 screenSize)
{
	//Clear textures due to discard in shader
	float clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->ClearRenderTargetView(this->rTVA[EdgeDetection], clear);
	deviceContext->ClearRenderTargetView(this->rTVA[BlendingWeight], clear);

	//Update constant buffer
	
		HRESULT hr = S_OK;

		SMAABuffer* dataPtr = (SMAABuffer*)this->smaaBuffer->Map(deviceContext);

		dataPtr->texelSize.x = 1.0f / (float)screenSize.x;
		dataPtr->texelSize.y = 1.0f / (float)screenSize.y;
		dataPtr->dummy = DirectX::XMFLOAT2(0.0f, 0.0f);

		this->smaaBuffer->Unmap(deviceContext);
		this->smaaBuffer->ApplyBuffer(deviceContext, 0, 0);
	

	//Edge detection
	{
		//Set Render target
		//Textures cannot be bound to both texture and shader at the same time.
		ID3D11ShaderResourceView* nullSrv = NULL;
		for (int i = 0; i<BUFFER_COUNT; i++)
		{
			deviceContext->PSSetShaderResources(i, 1, &nullSrv);
		}
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, this->backBufferDSV);
		//deviceContext->OMSetRenderTargets(1, &this->rTVA[EdgeDetection], this->backBufferDSV);

		deviceContext->PSSetShaderResources(0, 1, &backbuffer);
		this->edgeDetectionShaders->RenderIndexed(deviceContext, samplerState, 6);
	}
	/*
	// Blending Weight
	{
		//Set Render target
		//Textures cannot be bound to both texture and shader at the same time.
		ID3D11ShaderResourceView* nullSrv = NULL;
		for (int i = 0; i<BUFFER_COUNT; i++)
		{
			deviceContext->PSSetShaderResources(i, 1, &nullSrv);
		}
		//deviceContext->OMSetRenderTargets(1, &backBufferRTV, this->backBufferDSV);
		deviceContext->OMSetRenderTargets(1, &this->rTVA[BlendingWeight], this->backBufferDSV);
		this->smaaBuffer->ApplyBuffer(deviceContext, 0, 0);
		deviceContext->PSSetShaderResources(0, 1, &backbuffer);
		deviceContext->PSSetShaderResources(1, 1, &this->sRVA[EdgeDetection]);
		deviceContext->PSSetShaderResources(2, 1, &this->areaTexSRV);
		deviceContext->PSSetShaderResources(3, 1, &this->searchTexSRV);

		deviceContext->PSSetSamplers(0, 1, &samplerState);
		deviceContext->PSSetSamplers(1, 1, &this->pointSamplerState);
		this->blendingWeightShaders->RenderIndexed(deviceContext, samplerState, 6);
	}
	//Neighbourhood blending
	{
		// Set Render target
		//Textures cannot be bound to both texture and shader at the same time.
		ID3D11ShaderResourceView* nullSrv = NULL;
		for (int i = 0; i<BUFFER_COUNT; i++)
		{
			deviceContext->PSSetShaderResources(i, 1, &nullSrv);
		}
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, this->backBufferDSV);
		this->smaaBuffer->ApplyBuffer(deviceContext, 0, 0);
		deviceContext->PSSetShaderResources(0, 1, &backbuffer);
		deviceContext->PSSetShaderResources(1, 1, &this->sRVA[BlendingWeight]);
		this->neighbourhoodBlendingShaders->RenderIndexed(deviceContext, samplerState, 6);
	}*/
}

void SMAA::CreateRenderTargets(ID3D11Device* device, DirectX::XMINT2 screenSize)
{
	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = screenSize.x;
	textureDesc.Height = screenSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//Render target textures for the deferred shader to write to
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = device->CreateTexture2D(&textureDesc, NULL, &this->rTTA[i]);
		if (FAILED(hr))
			throw std::runtime_error("create RenderTargetTextureArray.SMAA");
	}

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	//
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = device->CreateRenderTargetView(this->rTTA[i], &renderTargetViewDesc, &this->rTVA[i]);
		if (FAILED(hr))
			throw std::runtime_error("create Render Target View. SMAA");
	}

	// Setup the description of the shader resource view. For the shaders to access the render target textures.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource views.

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = device->CreateShaderResourceView(this->rTTA[i], &shaderResourceViewDesc, &this->sRVA[i]);
		if (FAILED(hr))
			throw std::runtime_error("create Render Target View. SMAA");
	}
}

ID3D11ShaderResourceView* SMAA::CreateTextureFromRAWMemory(ID3D11Device* device, int width, int height, int pitch, int dataSize, DXGI_FORMAT format, const unsigned char * data)
{
	HRESULT hr = S_OK;
	// Create working textures
	D3D11_TEXTURE2D_DESC td;
	memset(&td, 0, sizeof(td));
	td.ArraySize = 1;
	td.Format = format;
	td.Height = height;
	td.Width = width;
	td.CPUAccessFlags = 0;
	td.MipLevels = 1;
	td.MiscFlags = 0;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA srData;
	srData.pSysMem = data;
	srData.SysMemPitch = pitch;
	srData.SysMemSlicePitch = 0;

	ID3D11Texture2D * tex;
	hr = device->CreateTexture2D(&td, &srData, &tex);
	if (FAILED(hr))
		throw std::runtime_error("CreateTexture2D. SMAA");

	ID3D11ShaderResourceView * ret = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = td.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(tex, &shaderResourceViewDesc, &ret);
	if (FAILED(hr))
		throw std::runtime_error("CreateTexture2D. SMAA");
	
	SAFE_RELEASE(tex);
	return ret;
}

void SMAA::CreateSampler(ID3D11Device* device)
{
	//Create SamplerState
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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
	hr = device->CreateSamplerState(&samplerDesc, &this->pointSamplerState);
	if (FAILED(hr))
		throw std::runtime_error("Create SamplerState - mSamplerState");
}
