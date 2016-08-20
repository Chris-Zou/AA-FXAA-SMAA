#include "Buffer.h"
#include "stdafx.h"
#include <stdexcept>


Buffer::Buffer(ID3D11Device* _device, BUFFER_DESCRIPTION _bd)
{
	HRESULT hr = S_OK;

	mBuffer = nullptr;

	D3D11_BUFFER_DESC bufferDesc;

	this->mUsage = _bd.Usage;
	this->mType = _bd.Type;
	this->mNumElem = _bd.NumElem;
	this->mVertexSize = _bd.TypeSize;

	switch (this->mType)
	{
	case VERTEX_BUFFER:
	{
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		if (_bd.Usage == BUFFER_STREAM_OUT_TARGET)
			bufferDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
		break;
	}
	case INDEX_BUFFER:
	{
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		break;
	}
	case CONSTANT_BUFFER_VS:
	{
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		break;
	}
	case CONSTANT_BUFFER_GS:
	{
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		break;
	}
	case CONSTANT_BUFFER_PS:
	{
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		break;
	}
	case CONSTANT_BUFFER_VSPS:
	{
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		break;
	}
	default:
	{
		throw std::runtime_error("Error buffer");
		break;
	}
	};

	//Default values
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = _bd.TypeSize * _bd.NumElem;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	switch (this->mUsage)
	{
	case BUFFER_DEFAULT:
	{
		//default values
		break;
	}
	case BUFFER_STREAM_OUT_TARGET:
	{
		//...
		break;
	}
	case BUFFER_CPU_WRITE:
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case BUFFER_CPU_WRITE_DISCARD:
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case BUFFER_CPU_READ:
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		break;
	}
	default:
	{
		throw std::runtime_error("Error buffer");
		break;
	}
	};

	//set at least 16 bytes
	if (bufferDesc.ByteWidth < 16)
		bufferDesc.ByteWidth = 16;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = _bd.InitData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	switch (this->mType)
	{
	case VERTEX_BUFFER:
	{
		hr = _device->CreateBuffer(&bufferDesc, &data, &this->mBuffer);
		break;
	}
	case INDEX_BUFFER:
	{
		hr = _device->CreateBuffer(&bufferDesc, &data, &this->mBuffer);
		break;
	}
	case CONSTANT_BUFFER_VS:
	{
		hr = _device->CreateBuffer(&bufferDesc, NULL, &this->mBuffer);
		break;
	}
	case CONSTANT_BUFFER_GS:
	{
		hr = _device->CreateBuffer(&bufferDesc, NULL, &this->mBuffer);
		break;
	}
	case CONSTANT_BUFFER_PS:
	{
		hr = _device->CreateBuffer(&bufferDesc, NULL, &this->mBuffer);
		break;
	}
	case CONSTANT_BUFFER_VSPS:
	{
		hr = _device->CreateBuffer(&bufferDesc, NULL, &this->mBuffer);
		break;
	}
	default:
	{
		throw std::runtime_error("Error buffer");
		break;
	}
	};

	if (FAILED(hr))
		throw std::runtime_error("Error buffer");
}

Buffer::~Buffer() 
{
	SAFE_RELEASE(this->mBuffer);
}

void* Buffer::Map(ID3D11DeviceContext* _deviceContext)
{
	void* ret = nullptr;
	if (this->mUsage == BUFFER_CPU_WRITE || this->mUsage == BUFFER_CPU_READ || this->mUsage == BUFFER_CPU_WRITE_DISCARD)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		UINT32 mapType = 0;

		if (this->mUsage == BUFFER_CPU_READ)
			mapType = D3D11_MAP_READ;
		else if (this->mUsage == BUFFER_CPU_WRITE)
			mapType = D3D11_MAP_WRITE;
		else if (this->mUsage == BUFFER_CPU_WRITE_DISCARD)
			mapType = D3D11_MAP_WRITE_DISCARD;

		HRESULT hr = S_OK;

		hr = _deviceContext->Map(this->mBuffer, NULL, (D3D11_MAP)mapType, NULL, &MappedResource);
		if (FAILED(hr))
			ret = NULL;
		else
			ret = MappedResource.pData;
	}

	return ret;
}

void Buffer::Unmap(ID3D11DeviceContext* _deviceContext)
{
	_deviceContext->Unmap(this->mBuffer, 0);
}

ID3D11Buffer* Buffer::GetBuffer()const
{
	return this->mBuffer;
}

void Buffer::ApplyBuffer(ID3D11DeviceContext* _deviceContext, unsigned int _stride, unsigned int _offset)
{
	if (this->mType == VERTEX_BUFFER)
		_deviceContext->IASetVertexBuffers(0, 1, &this->mBuffer, &_stride, &_offset);
	else if (this->mType == INDEX_BUFFER)
		_deviceContext->IASetIndexBuffer(this->mBuffer, DXGI_FORMAT_R32_UINT, _offset);
	else if (this->mType == CONSTANT_BUFFER_VS)
		_deviceContext->VSSetConstantBuffers(0, 1, &this->mBuffer);
	else if (this->mType == CONSTANT_BUFFER_PS)
		_deviceContext->PSSetConstantBuffers(0, 1, &this->mBuffer);
	else if (this->mType == CONSTANT_BUFFER_VSPS)
	{
		_deviceContext->VSSetConstantBuffers(0, 1, &this->mBuffer);
		_deviceContext->PSSetConstantBuffers(0, 1, &this->mBuffer);
	}
}

UINT32 Buffer::GetNumElem()const
{
	return this->mNumElem;
}

UINT32 Buffer::GetVertexSize()const
{
	return this->mVertexSize;
}