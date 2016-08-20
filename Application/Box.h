#pragma once

#include <string>
#include <DirectXMath.h>
#include <windows.h>
#include <d3d11.h>
#include "include/D3DX11tex.h"
#pragma comment (lib, "libs/d3dx11.lib")

#include "Buffer.h"

__declspec(align(16))class Box
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Texture;
	};

private:
	ID3D11ShaderResourceView*	mTexture;
	Buffer* mVertexBuffer;
	Buffer* mIndexBuffer;
	DirectX::XMMATRIX _localSpace;
	int _numIndex;

private:
	void InitBuffers(ID3D11Device* device);

public:
	Box(ID3D11Device* device, std::wstring filePath, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);
	~Box();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetNumIndex()const;
	ID3D11ShaderResourceView* GetTexture()const;
	DirectX::XMMATRIX* GetLocalSpace();

	void SetBox(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT3 sca);
	//Overloading these guarantees 16B alignment of XMMATRIX
	void* operator new(size_t i);
	void operator delete(void* p);
};