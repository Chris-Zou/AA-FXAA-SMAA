#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <fstream>
#include <DirectXMath.h>
#include "stdafx.h"
#include "include/D3DX11tex.h"
#pragma comment (lib, "libs/d3dx11.lib")


__declspec(align(16)) class ObjObject
{
private:
	struct Vec2
	{
		float x;
		float y;
	};

	struct Vec3
	{
		float x;
		float y;
		float z;
	};
	struct Vertex
	{
		Vec3 position;
		Vec3 normal;
		Vec2 texCoord;
	};
	struct MTL
	{
		std::string newmtl;
		float illum;
		Vec3 Kd;
		Vec3 Ka;
		Vec3 Tf;
		std::string textureMap;
		float Ni;
	};
	struct Group
	{
		std::string groupName;
		std::string useMtl;
	};

private:
	ID3D11Device*				mDevice;
	ID3D11DeviceContext*		mDeviceContext;

private:
	DirectX::XMMATRIX			mModelSpace;
	DirectX::XMFLOAT3			mPosition;
	DirectX::XMFLOAT3			mRotation;
	DirectX::XMFLOAT3			mScale;

	ID3D11Buffer*				mVertexBuffer;
	ID3D11Buffer*				mIndexBuffer;
	ID3D11Buffer*				mLightBuffer;

	ID3D11SamplerState*			mSamplerState;
	ID3D11ShaderResourceView*	mTexture;

	int							mNrOfVertex;
	
private:
	HRESULT						CreateBuffers(std::vector<Vertex> &vertexBufferData);
	HRESULT						CreateMTL(std::string mtllib, std::vector<Group> groups, std::vector<MTL> &mtls);
	HRESULT						CreateTexture(std::vector<MTL> &mtls);
	
public:
	ObjObject(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~ObjObject();
	
	HRESULT						LoadFile(std::string filename);  
	void						Render();

	void SetObject(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);
	int	GetNumVertex()const;
	ID3D11ShaderResourceView*	GetTexture();
	DirectX::XMMATRIX*			GetModelSpace();
	void						Rotate(DirectX::XMFLOAT3 rotDelta);

	void* operator new(size_t i)
	{
		return _mm_malloc(i,16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
};