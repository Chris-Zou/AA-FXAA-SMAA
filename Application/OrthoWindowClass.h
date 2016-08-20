#pragma once
#include "Buffer.h"
#include <DirectXMath.h>
#include <stdexcept>

class OrthoWindowClass
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
	};

	Buffer*				mVertexBuffer;
	Buffer*				mIndexBuffer;
	UINT32				mNumIndex;
	DirectX::XMINT2		mScreenSize;

public:
	OrthoWindowClass(ID3D11Device* device, DirectX::XMINT2 screenSize);
	~OrthoWindowClass();

	void Render(ID3D11DeviceContext* deviceContext);
	UINT32 GetIndexCount()const;
	DirectX::XMINT2 GetScreenSize()const;
};