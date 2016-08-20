#include "OrthoWindowClass.h"
using namespace DirectX;

OrthoWindowClass::OrthoWindowClass(ID3D11Device* _device, XMINT2 _screenSize)
{
	HRESULT hr = S_OK;

	const UINT32	numVertex = 4;
	const UINT32	numIndex = 6;
	this->mNumIndex = numIndex;
	this->mScreenSize = _screenSize;

	float left = -1;
	float right = 1;
	float top = 1;
	float bottom = -1;

	/*
	float left = (float)((_screenSize.x / 2) * -1);
	float right = left + (float)_screenSize.x;
	float top = (float)(_screenSize.y / 2);
	float bottom = top - (float)_screenSize.y;
	*/

	Vertex			vertices[numVertex];
	memset(vertices, 0, (sizeof(Vertex)*numVertex));
	vertices[0].Position = XMFLOAT3(left, bottom, 0.0f);  // Top left.
	vertices[0].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[1].Position = XMFLOAT3(left, top, 0.0f);  // Bottom left.
	vertices[1].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertices[2].Position = XMFLOAT3(right, bottom, 0.0f);  // Top right.
	vertices[2].TexCoord = XMFLOAT2(1.0f, 1.0f);

	vertices[3].Position = XMFLOAT3(right, top, 0.0f);  // Bottom right.
	vertices[3].TexCoord = XMFLOAT2(1.0f, 0.0f);

	unsigned long	indices[numIndex] =
	{
		0,1,2,
		2,1,3
	};

	BUFFER_DESCRIPTION vbd, ibd;

	vbd.InitData = vertices;
	vbd.NumElem = numVertex;
	vbd.Type = VERTEX_BUFFER;
	vbd.Usage = BUFFER_CPU_WRITE;
	vbd.TypeSize = sizeof(Vertex);

	this->mVertexBuffer = new Buffer(_device, vbd);


	ibd.InitData = indices;
	ibd.NumElem = numIndex;
	ibd.Type = INDEX_BUFFER;
	ibd.Usage = BUFFER_DEFAULT;
	ibd.TypeSize = sizeof(unsigned long);

	this->mIndexBuffer = new Buffer(_device, ibd);
}

OrthoWindowClass::~OrthoWindowClass()
{
	delete this->mVertexBuffer;
	delete this->mIndexBuffer;
}

void OrthoWindowClass::Render(ID3D11DeviceContext* _deviceContext)
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;

	this->mVertexBuffer->ApplyBuffer(_deviceContext, stride, offset);
	this->mIndexBuffer->ApplyBuffer(_deviceContext, 0, 0);
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

UINT32 OrthoWindowClass::GetIndexCount()const
{
	return this->mNumIndex;
}

DirectX::XMINT2 OrthoWindowClass::GetScreenSize()const
{
	return this->mScreenSize;
}