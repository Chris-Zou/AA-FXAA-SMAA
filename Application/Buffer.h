#pragma once

#include <d3d11.h>

enum BUFFER_TYPE
{
	VERTEX_BUFFER,
	INDEX_BUFFER,
	CONSTANT_BUFFER_VS,
	CONSTANT_BUFFER_GS,
	CONSTANT_BUFFER_PS,
	CONSTANT_BUFFER_VSPS,
	BUFFER_TYPE_COUNT
};

enum BUFFER_USAGE
{
	BUFFER_DEFAULT,
	BUFFER_STREAM_OUT_TARGET,
	BUFFER_CPU_WRITE,
	BUFFER_CPU_WRITE_DISCARD,
	BUFFER_CPU_READ,
	BUFFER_USAGE_COUNT
};

struct BUFFER_DESCRIPTION
{
	BUFFER_TYPE		Type;
	BUFFER_USAGE	Usage;
	UINT32			NumElem;
	UINT32			TypeSize; //sizeof
	void*			InitData;
	BUFFER_DESCRIPTION()
	{
		InitData = nullptr;
	}
};

class Buffer
{
private:
	BUFFER_TYPE			mType;
	BUFFER_USAGE		mUsage;
	UINT32				mNumElem;
	UINT32				mVertexSize;
	ID3D11Buffer*		mBuffer;

public:
	Buffer(ID3D11Device* device, BUFFER_DESCRIPTION bufferDescription);
	~Buffer();

	void* Map(ID3D11DeviceContext* deviceContext);
	void Unmap(ID3D11DeviceContext* deviceContext);

	ID3D11Buffer*	GetBuffer()const;
	void			ApplyBuffer(ID3D11DeviceContext* deviceContext, unsigned int stride, unsigned int offset);
	UINT32			GetNumElem()const;
	UINT32			GetVertexSize()const;


};