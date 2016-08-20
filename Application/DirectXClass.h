#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")


enum CullMode
{
	None,
	Front,
	Back
};

class DirectXClass
{
private:
	static const int			BUFFER_COUNT = 3;
	// 0. Color
	// 1. Normals
	// 2. BackBuffer
private:
	ID3D11Device*				mDevice;
	ID3D11DeviceContext*		mDeviceContext;
	IDXGISwapChain*				mSwapChain;


	ID3D11RasterizerState*		mRasterStateNone;
	ID3D11RasterizerState*		mRasterStateFront;
	ID3D11RasterizerState*		mRasterStateBack;

	//Deferred - FXAA
	ID3D11Texture2D*			mRenderTargetTextureArray[BUFFER_COUNT];
	ID3D11RenderTargetView*		mRenderTargetViewArray[BUFFER_COUNT];
	ID3D11ShaderResourceView*	mShaderResourceViewArray[BUFFER_COUNT];
	//
	//Deferred - Normal and SMAA
	ID3D11Texture2D*			mRTTA[BUFFER_COUNT];
	ID3D11RenderTargetView*		mRTVA[BUFFER_COUNT];
	ID3D11ShaderResourceView*	mSRVA[BUFFER_COUNT];
	//
	//BackBuffer
	ID3D11RenderTargetView*		mRenderTargetView;
	ID3D11Texture2D*			mDepthStencil;
	ID3D11DepthStencilView*		mDepthStencilView;

	ID3D11DepthStencilState*	mDepthStencilState;
	ID3D11DepthStencilState*	mDepthDisabledStencilState;

	ID3D11BlendState*			mAlphaEnableBlendingState;
	ID3D11BlendState*			mAlphaDisableBlendingState;


	ID3D11SamplerState*			mSamplerState;
	D3D11_VIEWPORT				mViewport;

private:
	void Initialize(DirectX::XMINT2 screenSize, HWND hwnd);

public:
	DirectXClass(DirectX::XMINT2 screenSize, HWND hwnd);
	~DirectXClass();

	ID3D11Device* GetDevice()const;
	ID3D11DeviceContext* GetDeviceContext()const;
	void ClearBackBuffer(DirectX::XMFLOAT4 color);
	void SwapChain();
	void ClearShaderResource();
	ID3D11SamplerState*			GetSamplerState()const;
	ID3D11RasterizerState*		GetRasterState(CullMode mode);
	
	//Get BackBuffer variables
	ID3D11RenderTargetView* GetBackBufferRTV();
	ID3D11DepthStencilView* GetBackBufferDSV();


	//if true NOFXAA/SMAA
	ID3D11ShaderResourceView* GetShaderResourceView(int, bool target);

	void SetDeferredRenderTargets();
	void SetFXAARenderTargets();
	//if true == NOFXAA/SMAA
	void SetLightRenderTarget(bool target);

	void ClearDeferredRenderTargets(DirectX::XMFLOAT4 color);
	void SetBackBufferAsRenderTarget();

	void ClearRender();

	void TurnZBufferOn();
	void TurnZBufferOff();
};

