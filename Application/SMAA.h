#include "Shader.h"
#include "Buffer.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include "Assets\Textures\AreaTex.h"
#include "Assets\Textures\SearchTex.h"

class SMAA
{
private:
	struct SMAABuffer
	{
		DirectX::XMFLOAT2 texelSize;
		DirectX::XMFLOAT2 dummy;
	};
	enum RenderTarget
	{
		EdgeDetection,
		BlendingWeight
	};

private:
	ID3D11RenderTargetView* backBufferRTV;
	ID3D11DepthStencilView* backBufferDSV;

	//Shaders
	Shader* edgeDetectionShaders;
	Shader* blendingWeightShaders;
	Shader* neighbourhoodBlendingShaders;
	//Buffers
	Buffer* smaaBuffer;

	//RenderTargets etc
	void CreateRenderTargets(ID3D11Device* device, DirectX::XMINT2 screenSize);
	static const int BUFFER_COUNT = 2;
	ID3D11Texture2D*			rTTA[BUFFER_COUNT];
	ID3D11RenderTargetView*		rTVA[BUFFER_COUNT];
	ID3D11ShaderResourceView*	sRVA[BUFFER_COUNT];

	//SMAA Textures
	ID3D11ShaderResourceView* areaTexSRV;
	ID3D11ShaderResourceView* searchTexSRV;
	ID3D11ShaderResourceView* CreateTextureFromRAWMemory(ID3D11Device* pD3DDevice, int width, int height, int pitch, int dataSize, DXGI_FORMAT format, const unsigned char * data);
	ID3D11SamplerState* pointSamplerState;
	void CreateSampler(ID3D11Device* device);

public:
	SMAA(ID3D11Device* device, DirectX::XMINT2 screenSize, ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV);
	~SMAA();
	void Render(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* backbuffer, DirectX::XMINT2	screenSize);
};