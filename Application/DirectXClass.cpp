#include "DirectXClass.h"
#include "stdafx.h"
#include <stdexcept>

DirectXClass::DirectXClass(DirectX::XMINT2 screenSize, HWND hwnd)
{
	Initialize(screenSize, hwnd);
}

DirectXClass::~DirectXClass()
{
	SAFE_RELEASE(mDevice);
	SAFE_RELEASE(mDeviceContext);
	SAFE_RELEASE(mSwapChain);
	for (int i = 0; i<BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(this->mRenderTargetTextureArray[i]);
		SAFE_RELEASE(this->mRenderTargetViewArray[i]);
		SAFE_RELEASE(this->mShaderResourceViewArray[i]);
	}
	SAFE_RELEASE(mRenderTargetView);
	SAFE_RELEASE(mDepthStencil);
	SAFE_RELEASE(mDepthStencilView);

	SAFE_RELEASE(mDepthDisabledStencilState);
	SAFE_RELEASE(mAlphaEnableBlendingState);
	SAFE_RELEASE(mAlphaDisableBlendingState);
}

void DirectXClass::Initialize(DirectX::XMINT2 screenSize, HWND hwnd)
{
	HRESULT hr = E_FAIL;

	//Swap Chain
	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = screenSize.x;
	sd.BufferDesc.Height = screenSize.y;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS; // | DXGI_USAGE_UNORDERED_ACCESS is for compute shader.
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	
	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE };
	D3D_FEATURE_LEVEL featureLevelsToTry[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL initiatedFeatureLevel;

	for ( UINT driverTypeIndex = 0; driverTypeIndex < ARRAYSIZE( driverTypes ) && FAILED( hr ); driverTypeIndex++ )
	{
		hr = D3D11CreateDeviceAndSwapChain( nullptr,
											driverTypes[driverTypeIndex],
											NULL,
											0,
											featureLevelsToTry,
											ARRAYSIZE( featureLevelsToTry ),
											D3D11_SDK_VERSION,
											&sd,
											&mSwapChain,
											&mDevice,
											&initiatedFeatureLevel,
											&mDeviceContext );
	}

	if (FAILED(hr))
		throw std::runtime_error("D3D11CreateDeviceAndSwapChain");

	//Create RasterState
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	hr =  mDevice->CreateRasterizerState(&rasterDesc, &mRasterStateNone);
	if (FAILED(hr))
		throw std::runtime_error("CreateRasterizerState - mRasterStateNone");

	rasterDesc.CullMode = D3D11_CULL_FRONT;
	hr = mDevice->CreateRasterizerState(&rasterDesc, &mRasterStateFront);
	if (FAILED(hr))
		throw std::runtime_error("CreateRasterizerState - mRasterStateFront");

	rasterDesc.CullMode = D3D11_CULL_BACK;
	hr = mDevice->CreateRasterizerState(&rasterDesc, &mRasterStateBack);
	if (FAILED(hr))
		throw std::runtime_error("CreateRasterizerState - mRasterStateBack");

	mDeviceContext->RSSetState(mRasterStateNone);

	//Deferred//
	//RenderTargetTextureArray

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
		hr = this->mDevice->CreateTexture2D(&textureDesc, NULL, &this->mRTTA[i]);
		if (FAILED(hr))
			throw std::runtime_error("create RenderTargetTextureArray. DeferredBufferClass Texture2D");
	}

		hr = this->mDevice->CreateTexture2D(&textureDesc, NULL, &this->mRenderTargetTextureArray[1]);
		if (FAILED(hr))
			throw std::runtime_error("create RenderTargetTextureArray. DeferredBufferClass Texture2D");

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	//
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = this->mDevice->CreateRenderTargetView(this->mRTTA[i], &renderTargetViewDesc, &this->mRTVA[i]);
		if (FAILED(hr))
			throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetView");
	}
	//

	// Create the render target views. To access the render target textures through
	hr = this->mDevice->CreateRenderTargetView(this->mRenderTargetTextureArray[1], &renderTargetViewDesc, &this->mRenderTargetViewArray[1]);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetView");

	// Setup the description of the shader resource view. For the shaders to access the render target textures.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource views.
	
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		hr = this->mDevice->CreateShaderResourceView(this->mRTTA[i], &shaderResourceViewDesc, &this->mSRVA[i]);
		if (FAILED(hr))
			throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetViewcreate Shader Resource View. DeferredBufferClass CreateShaderResourceView");
	}

	hr = this->mDevice->CreateShaderResourceView(this->mRenderTargetTextureArray[1], &shaderResourceViewDesc, &this->mShaderResourceViewArray[1]);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetViewcreate Shader Resource View. DeferredBufferClass CreateShaderResourceView");

	//FXAA
	//Edit color
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	hr = this->mDevice->CreateTexture2D(&textureDesc, NULL, &this->mRenderTargetTextureArray[0]);
	if (FAILED(hr))
		throw std::runtime_error("create RenderTargetTextureArray. DeferredBufferClass Texture2D");
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	hr = this->mDevice->CreateRenderTargetView(this->mRenderTargetTextureArray[0], &renderTargetViewDesc, &this->mRenderTargetViewArray[0]);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetView");
	shaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	hr = this->mDevice->CreateShaderResourceView(this->mRenderTargetTextureArray[0], &shaderResourceViewDesc, &this->mShaderResourceViewArray[0]);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetViewcreate Shader Resource View. DeferredBufferClass CreateShaderResourceView");

	//Create Shaderresourceview and target for FXAA
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	hr = this->mDevice->CreateTexture2D(&textureDesc, NULL, &this->mRenderTargetTextureArray[2]);
	if (FAILED(hr))
		throw std::runtime_error("create RenderTargetTextureArray FXAA. DeferredBufferClass Texture2D");


	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	hr = this->mDevice->CreateRenderTargetView(this->mRenderTargetTextureArray[2], &renderTargetViewDesc, &this->mRenderTargetViewArray[2]);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetView");

	shaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	hr = this->mDevice->CreateShaderResourceView(this->mRenderTargetTextureArray[2], &shaderResourceViewDesc, &this->mShaderResourceViewArray[2]);
	if (FAILED(hr))
		throw std::runtime_error("create Render Target View. DeferredBufferClass CreateRenderTargetViewcreate Shader Resource View. DeferredBufferClass CreateShaderResourceView");



	//Create RenderTargetView
	ID3D11Texture2D* pBackBuffer;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr))
		throw std::runtime_error("Create RenderTargetView - mSwapChain->GetBuffer");

	hr = mDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mRenderTargetView);
	if (FAILED(hr))
		throw std::runtime_error("CreateRenderTargetView");

	pBackBuffer->Release();

	//Create DepthStencil
	D3D11_TEXTURE2D_DESC dsd;
	ZeroMemory(&dsd, sizeof(dsd));
	dsd.Width = screenSize.x;
	dsd.Height = screenSize.y;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsd.SampleDesc.Count = 1;
	dsd.SampleDesc.Quality = 0;
	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsd.CPUAccessFlags = 0;
	dsd.MiscFlags = 0;

	hr = mDevice->CreateTexture2D(&dsd, nullptr, &mDepthStencil);
	if (FAILED(hr))
		throw std::runtime_error("Create DepthStencil - CreateTexture2D");

	//Create DepthStencilView
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	hr =  mDevice->CreateDepthStencilView(mDepthStencil, &dsvd, &mDepthStencilView);
	if (FAILED(hr))
		throw std::runtime_error("Create DepthStencil - CreateTexture2D");

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	//Create Depth stencil state
	// Set up the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	hr =  mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
	if (FAILED(hr))
		throw std::runtime_error("Create DepthStencilState - CreateDepthStencilState");

	//Create Depth disabled stencil state
	depthStencilDesc.DepthEnable = false;

	hr =  mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthDisabledStencilState);
	if (FAILED(hr))
		throw std::runtime_error("Create DepthDisabledStencilState - CreateDepthStencilState");

	//Create Alpha blending state
	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	hr =  mDevice->CreateBlendState(&blendStateDescription, &mAlphaEnableBlendingState);
	if (FAILED(hr))
		throw std::runtime_error("Create Create Alpha blending state - mAlphaEnableBlendingState");

	//Create Alpha Disabled blending state
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	hr =  mDevice->CreateBlendState(&blendStateDescription, &mAlphaDisableBlendingState);
	if (FAILED(hr))
		throw std::runtime_error("Create Create Alpha blending state - mAlphaDisableBlendingState");

	//Create SamplerState
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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


	hr = mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);
	if (FAILED(hr))
		throw std::runtime_error("Create SamplerState - mSamplerState");

	//Create viewport
	mViewport.Width = (float)screenSize.x;
	mViewport.Height = (float)screenSize.y;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mDeviceContext->RSSetViewports(1, &mViewport);
}

ID3D11Device * DirectXClass::GetDevice() const
{
	return mDevice;
}

ID3D11DeviceContext * DirectXClass::GetDeviceContext() const
{
	return mDeviceContext;
}


void DirectXClass::ClearBackBuffer(DirectX::XMFLOAT4 color)
{
	// Clear Back Buffer
	static float clearColor[4] = { color.x, color.y, color.z, color.w};
	mDeviceContext->ClearRenderTargetView(this->mRenderTargetView, clearColor);

	// Clear Depth Buffer
	mDeviceContext->ClearDepthStencilView(this->mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DirectXClass::SwapChain()
{
	mSwapChain->Present(0, 0);
}

void DirectXClass::ClearShaderResource()
{
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	mDeviceContext->PSSetShaderResources(0, 2, null);
}

ID3D11SamplerState* DirectXClass::GetSamplerState()const
{
	return mSamplerState;
}

ID3D11RasterizerState* DirectXClass::GetRasterState(CullMode mode)
{
	if (mode == Back)
		return mRasterStateBack;
	else if(mode == Front)
		return mRasterStateFront;
	else
		return mRasterStateNone;
}

ID3D11RenderTargetView * DirectXClass::GetBackBufferRTV()
{
	return mRenderTargetView;
}

ID3D11DepthStencilView * DirectXClass::GetBackBufferDSV()
{
	return mDepthStencilView;
}

void DirectXClass::SetDeferredRenderTargets()
{
	//Textures cannot be bound to both texture and shader at the same time.
	ID3D11ShaderResourceView* nullSrv = NULL;
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		mDeviceContext->PSSetShaderResources(i, 1, &nullSrv);
	}

	// Bind the render target view array and depth stencil buffer to the output render pipeline.
	mDeviceContext->OMSetRenderTargets(BUFFER_COUNT - 1, this->mRTVA, this->mDepthStencilView);

	// Set the viewport.
	mDeviceContext->RSSetViewports(1, &this->mViewport);

}

void DirectXClass::SetFXAARenderTargets()
{
	//Textures cannot be bound to both texture and shader at the same time.
	ID3D11ShaderResourceView* nullSrv = NULL;
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		mDeviceContext->PSSetShaderResources(i, 1, &nullSrv);
	}

	// Bind the render target view array and depth stencil buffer to the output render pipeline.
	mDeviceContext->OMSetRenderTargets(BUFFER_COUNT - 1, this->mRenderTargetViewArray, this->mDepthStencilView);

	// Set the viewport.
	mDeviceContext->RSSetViewports(1, &this->mViewport);
}

void DirectXClass::SetLightRenderTarget(bool target)
{
	//Textures cannot be bound to both texture and shader at the same time.
	ID3D11ShaderResourceView* nullSrv = NULL;
	for (int i = 0; i<BUFFER_COUNT; i++)
	{
		mDeviceContext->PSSetShaderResources(i, 1, &nullSrv);
	}

	// Bind the render target view array and depth stencil buffer to the output render pipeline.
	if(target)
		mDeviceContext->OMSetRenderTargets(1, &this->mRTVA[2], this->mDepthStencilView); 
	else
		mDeviceContext->OMSetRenderTargets(1, &this->mRenderTargetViewArray[2], this->mDepthStencilView);

	// Set the viewport.
	mDeviceContext->RSSetViewports(1, &this->mViewport);
}

void DirectXClass::ClearDeferredRenderTargets(DirectX::XMFLOAT4 color)
{
	static float clearColor[4] = { color.x, color.y, color.z, color.w };

	// Clear the render target buffers.
	for (int i = 0; i<BUFFER_COUNT; i++)
	{
		mDeviceContext->ClearRenderTargetView(this->mRenderTargetViewArray[i], clearColor);
		mDeviceContext->ClearRenderTargetView(this->mRTVA[i], clearColor);
	}

	// Clear the depth buffer.
	mDeviceContext->ClearDepthStencilView(this->mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DirectXClass::SetBackBufferAsRenderTarget()
{
	this->mDeviceContext->OMSetRenderTargets(1, &this->mRenderTargetView, this->mDepthStencilView);
}

void DirectXClass::ClearRender()
{
	//no input layout
	this->mDeviceContext->IASetInputLayout(NULL);
	this->mDeviceContext->IASetVertexBuffers(NULL, NULL, NULL, NULL, NULL);
	this->mDeviceContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, NULL);

	//no vs
	this->mDeviceContext->PSSetShaderResources(NULL, NULL, NULL);
	this->mDeviceContext->VSSetShader(NULL, NULL, NULL);
	this->mDeviceContext->VSSetConstantBuffers(NULL, NULL, NULL);
	//no ps
	this->mDeviceContext->PSSetShader(NULL, NULL, NULL);
	this->mDeviceContext->PSSetShaderResources(NULL, NULL, NULL);
	this->mDeviceContext->PSSetConstantBuffers(NULL, NULL, NULL);
	//no sampler
	this->mDeviceContext->PSSetSamplers(NULL, NULL, NULL);
}

void DirectXClass::TurnZBufferOn()
{
	this->mDeviceContext->OMSetDepthStencilState(this->mDepthStencilState, 1);
}

void DirectXClass::TurnZBufferOff()
{
	this->mDeviceContext->OMSetDepthStencilState(this->mDepthDisabledStencilState, 1);
}

ID3D11ShaderResourceView* DirectXClass::GetShaderResourceView(int _i, bool target)
{
	if(target)
		return this->mSRVA[_i];
	else
		return this->mShaderResourceViewArray[_i];
}