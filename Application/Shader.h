#pragma once

#include <d3d11.h>
#include "stdafx.h"

struct SHADER_DESCRIPTION
{
	char*						vertexFilePath;
	char*						vertexHeader;
	D3D11_INPUT_ELEMENT_DESC*	inputDesc;
	UINT32						vertexNumElem;

	char*						pixelFilePath;
	char*						pixelHeader;
};

class Shader
{
private:
	ID3D11VertexShader*		mVertexShader;
	ID3D11PixelShader*		mPixelShader;
	ID3D11InputLayout*		mInputLayout;

private:
	HRESULT CompileShader(char* shaderFile, char* pEntrypoint, char* pTarget, D3D10_SHADER_MACRO* pDefines, ID3DBlob** pCompiledShader);

public:
	Shader();
	Shader(ID3D11Device* device, SHADER_DESCRIPTION shaderDescription);
	~Shader();

	void	CreatePixelShader(ID3D11Device* device, SHADER_DESCRIPTION shaderDescription);
	ID3D11VertexShader* GetVertexShader();
	ID3D11PixelShader*	GetPixelShader();
	ID3D11InputLayout*	GetInputLayout();
	void	RenderIndexed(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, int indexCount);
	void	RenderVertex(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, int vertexCount);


};