#include "Shader.h"
#include <d3dcompiler.h>
#include <string>
#include <fstream>

Shader::Shader()
{
	mVertexShader = nullptr;
	mPixelShader = nullptr;
	mInputLayout = nullptr;
}

Shader::Shader(ID3D11Device* _device, SHADER_DESCRIPTION _shaderDescription)
{
	HRESULT hr = S_OK;
	//Vertex Shader
	ID3DBlob* vs = nullptr;
	hr = CompileShader(_shaderDescription.vertexFilePath, _shaderDescription.vertexHeader, "vs_5_0", nullptr, &vs);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CompileShader vertex");

	hr = _device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &this->mVertexShader);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CreateVertexShader");

	hr = _device->CreateInputLayout(_shaderDescription.inputDesc,
		_shaderDescription.vertexNumElem,
		vs->GetBufferPointer(),
		vs->GetBufferSize(),
		&this->mInputLayout);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CreateInputLayout");

	SAFE_RELEASE(vs);

	//PixelShader
	ID3DBlob* ps = nullptr;
	hr = CompileShader(_shaderDescription.pixelFilePath, _shaderDescription.pixelHeader, "ps_5_0", nullptr, &ps);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CompileShader Pixel");
	hr = _device->CreatePixelShader(ps->GetBufferPointer(),
		ps->GetBufferSize(),
		nullptr,
		&this->mPixelShader);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CreatePixelShader");

	SAFE_RELEASE(ps);
}

Shader::~Shader() 
{
	SAFE_RELEASE(this->mVertexShader);
	SAFE_RELEASE(this->mPixelShader);
	SAFE_RELEASE(this->mInputLayout);
}


HRESULT Shader::CompileShader(char* shaderFile, char* pEntrypoint, char* pTarget, D3D10_SHADER_MACRO* pDefines, ID3DBlob** pCompiledShader)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS |
		D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_DEBUG;

	std::string shader_code;
	std::ifstream in(shaderFile, std::ios::in | std::ios::binary);

	if (in)
	{
		in.seekg(0, std::ios::end);
		shader_code.resize((unsigned int)in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&shader_code[0], shader_code.size());
		in.close();
	}

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile(shader_code.data(),
		shader_code.size(),
		NULL,
		pDefines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		pEntrypoint,
		pTarget,
		dwShaderFlags,
		NULL,
		pCompiledShader,
		&pErrorBlob);

	if (pErrorBlob)
	{
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		SAFE_RELEASE(pErrorBlob);
	}

	return hr;
}

void Shader::CreatePixelShader(ID3D11Device * device, SHADER_DESCRIPTION shaderDescription)
{
	HRESULT hr = S_OK;
	//PixelShader
	ID3DBlob* ps = nullptr;
	hr = CompileShader(shaderDescription.pixelFilePath, shaderDescription.pixelHeader, "ps_5_0", nullptr, &ps);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CompileShader Pixel");
	hr = device->CreatePixelShader(ps->GetBufferPointer(),
		ps->GetBufferSize(),
		nullptr,
		&this->mPixelShader);
	if (FAILED(hr))
		throw std::runtime_error("Shader.h CreatePixelShader");

	SAFE_RELEASE(ps);
}

ID3D11VertexShader * Shader::GetVertexShader()
{
	return mVertexShader;
}

ID3D11PixelShader * Shader::GetPixelShader()
{
	return mPixelShader;
}

ID3D11InputLayout * Shader::GetInputLayout()
{
	return mInputLayout;
}

void Shader::RenderIndexed(ID3D11DeviceContext* _deviceContext, ID3D11SamplerState* _samplerState, int _indexCount)
{
	_deviceContext->IASetInputLayout(this->mInputLayout);
	_deviceContext->VSSetShader(this->mVertexShader, NULL, 0);
	_deviceContext->PSSetShader(this->mPixelShader, NULL, 0);
	_deviceContext->PSSetSamplers(0, 1, &_samplerState);
	_deviceContext->DrawIndexed(_indexCount, 0, 0);
}

void Shader::RenderVertex(ID3D11DeviceContext* _deviceContext, ID3D11SamplerState* _samplerState, int vertexCount)
{
	_deviceContext->IASetInputLayout(this->mInputLayout);
	_deviceContext->VSSetShader(this->mVertexShader, NULL, 0);
	_deviceContext->PSSetShader(this->mPixelShader, NULL, 0);
	_deviceContext->PSSetSamplers(0, 1, &_samplerState);
	_deviceContext->Draw(vertexCount, 0);
}