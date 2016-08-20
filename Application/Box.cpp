#include "Box.h"
using namespace DirectX;
using namespace std;


Box::Box(ID3D11Device* device, wstring filePath, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	mTexture = nullptr;
	HRESULT hr = E_FAIL;

	InitBuffers(device);
	SetBox(position, rotation, scale);

	//Load in Texture
	LPCWSTR fpath = filePath.c_str();


	hr = D3DX11CreateShaderResourceViewFromFile(device, fpath, nullptr, nullptr, &this->mTexture, nullptr);
	if (FAILED(hr))
		throw std::runtime_error("D3DX11CreateShaderResourceViewFromFile");
}

Box::~Box()
{
	delete this->mVertexBuffer;
	delete this->mIndexBuffer;
}

void Box::Render(ID3D11DeviceContext * deviceContext)
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;

	this->mVertexBuffer->ApplyBuffer(deviceContext, stride, offset);
	this->mIndexBuffer->ApplyBuffer(deviceContext, 0, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int Box::GetNumIndex() const
{
	return _numIndex;
}

ID3D11ShaderResourceView * Box::GetTexture() const
{
	return mTexture;
}

DirectX::XMMATRIX* Box::GetLocalSpace()
{
	return &_localSpace;
}

void Box::InitBuffers(ID3D11Device* device)
{
	HRESULT hr = E_FAIL;
	// Vertex Buffer

	//1-3
	//0-2

	const int numVertex = 24;

	Vertex vertices[numVertex] = {
		// front - 0
		XMFLOAT3(-1, -1, -1),
		XMFLOAT3(0, 0, -1),
		XMFLOAT2(0, 1),

		// front - 1
		XMFLOAT3(-1, 1, -1),
		XMFLOAT3(0, 0, -1),
		XMFLOAT2(0, 0),

		// front - 2
		XMFLOAT3(1, -1, -1),
		XMFLOAT3(0, 0, -1),
		XMFLOAT2(1, 1),

		// front - 3
		XMFLOAT3(1, 1, -1),
		XMFLOAT3(0, 0, -1),
		XMFLOAT2(1, 0),

		// Right - 0
		XMFLOAT3(1, -1, -1),
		XMFLOAT3(1, 0, 0),
		XMFLOAT2(0, 1),

		// Right - 1
		XMFLOAT3(1, 1, -1),
		XMFLOAT3(1, 0, 0),
		XMFLOAT2(0, 0),

		// Right - 2
		XMFLOAT3(1, -1, 1),
		XMFLOAT3(1, 0, 0),
		XMFLOAT2(1, 1),

		// Right - 3
		XMFLOAT3(1, 1, 1),
		XMFLOAT3(1, 0, 0),
		XMFLOAT2(1, 0),

		// Far - 0
		XMFLOAT3(1, -1, 1),
		XMFLOAT3(0, 0, 1),
		XMFLOAT2(0, 1),

		// far - 1
		XMFLOAT3(1, 1, 1),
		XMFLOAT3(0, 0, 1),
		XMFLOAT2(0, 0),

		// far - 2
		XMFLOAT3(-1, -1, 1),
		XMFLOAT3(0, 0, 1),
		XMFLOAT2(1, 1),

		// far - 3
		XMFLOAT3(-1, 1, 1),
		XMFLOAT3(0, 0, 1),
		XMFLOAT2(1, 0),

		// Left - 0
		XMFLOAT3(-1, -1, 1),
		XMFLOAT3(-1, 0, 0),
		XMFLOAT2(0, 1),

		// Left - 1
		XMFLOAT3(-1, 1, 1),
		XMFLOAT3(-1, 0, 0),
		XMFLOAT2(0, 0),

		// Left - 2
		XMFLOAT3(-1, -1, -1),
		XMFLOAT3(-1, 0, 0),
		XMFLOAT2(1, 1),

		// Left - 3
		XMFLOAT3(-1, 1, -1),
		XMFLOAT3(-1, 0, 0),
		XMFLOAT2(1, 0),

		// Top - 0
		XMFLOAT3(-1, 1, -1),
		XMFLOAT3(0, 1, 0),
		XMFLOAT2(0, 1),

		// Top - 1
		XMFLOAT3(-1, 1, 1),
		XMFLOAT3(0, 1, 0),
		XMFLOAT2(0, 0),

		// Top - 2
		XMFLOAT3(1, 1, -1),
		XMFLOAT3(0, 1, 0),
		XMFLOAT2(1, 1),

		// Top - 3
		XMFLOAT3(1, 1, 1),
		XMFLOAT3(0, 1, 0),
		XMFLOAT2(1, 0),

		// Bottom - 0
		XMFLOAT3(-1, -1, -1),
		XMFLOAT3(0, -1, 0),
		XMFLOAT2(0, 1),

		// Bottom - 1
		XMFLOAT3(1, -1, -1),
		XMFLOAT3(0, -1, 0),
		XMFLOAT2(0, 0),

		// Bottom - 2
		XMFLOAT3(-1, -1, 1),
		XMFLOAT3(0, -1, 0),
		XMFLOAT2(1, 1),

		// Bottom - 3
		XMFLOAT3(1, -1, 1),
		XMFLOAT3(0, -1, 0),
		XMFLOAT2(1, 0)
	};

	BUFFER_DESCRIPTION vbd;

	vbd.Type = VERTEX_BUFFER;
	vbd.Usage = BUFFER_DEFAULT;
	vbd.NumElem = numVertex;
	vbd.TypeSize = sizeof(Vertex);
	vbd.InitData = vertices;

	this->mVertexBuffer = new Buffer(device, vbd);



	//Index Buffer
	const int numIndices = 72;
	_numIndex = numIndices;
	unsigned long indices[numIndices] = {
		//front
		0,1,2,
		2,1,3,
		4,5,6,
		6,5,7,
		//right
		8,9,10,
		10,9,11,
		12,13,14,
		14,13,15,
		//far
		16,17,18,
		18,17,19,
		20,21,22,
		22,21,23,
		//left
		24,25,26,
		26,25,27,
		28,29,30,
		30,29,31,
		//top
		32,33,34,
		34,33,35,
		36,37,38,
		38,37,39,
		//bottom
		40,41,42,
		42,41,43,
		44,45,46,
		46,45,47
	};

	BUFFER_DESCRIPTION ibd;

	ibd.Type = INDEX_BUFFER;
	ibd.Usage = BUFFER_DEFAULT;
	ibd.NumElem = numIndices;
	ibd.TypeSize = sizeof(unsigned long);
	ibd.InitData = indices;

	this->mIndexBuffer = new Buffer(device, ibd);
}

void Box::SetBox(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT3 sca)
{
	DirectX::XMMATRIX localSpace = XMMatrixIdentity();
	//Scale
	XMMATRIX S = XMMatrixScaling(sca.x, sca.y, sca.z);
	localSpace = XMMatrixMultiply(localSpace, S);
	//Rotation
	XMMATRIX R = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	//Position
	XMMATRIX P = XMMatrixTranslation(pos.x, pos.y, pos.z);
	//Combining into localSpace
	XMMATRIX tempSpace = XMMatrixMultiply(S, R);
	_localSpace = XMMatrixMultiply(tempSpace, P);
}

void Box::operator delete(void* p)
{
	_mm_free(p);
}

void* Box::operator new(size_t i)
{
	return _mm_malloc(i, 16);
}