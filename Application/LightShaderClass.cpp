#include "LightShaderClass.h"

LightShaderClass::LightShaderClass(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	//Shaders
	SHADER_DESCRIPTION sd;
	sd.vertexFilePath = "Assets/Shaders/LightVertexShader.hlsl";
	sd.vertexHeader = "VS_Light";
	sd.vertexNumElem = 2;
	sd.inputDesc = new D3D11_INPUT_ELEMENT_DESC[2];

	sd.inputDesc[0].SemanticName = "POSITION";
	sd.inputDesc[0].SemanticIndex = 0;
	sd.inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	sd.inputDesc[0].InputSlot = 0;
	sd.inputDesc[0].AlignedByteOffset = 0;
	sd.inputDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[0].InstanceDataStepRate = 0;

	sd.inputDesc[1].SemanticName = "TEXCOORD";
	sd.inputDesc[1].SemanticIndex = 0;
	sd.inputDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	sd.inputDesc[1].InputSlot = 0;
	sd.inputDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	sd.inputDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[1].InstanceDataStepRate = 0;

	sd.pixelFilePath = "Assets/Shaders/LightPixelShader.hlsl";
	sd.pixelHeader = "PS_Light";

	mVPShader = new Shader(device, sd);

	
	delete[]sd.inputDesc;

	//WVP Buffer
	BUFFER_DESCRIPTION bd;
	bd.Type = CONSTANT_BUFFER_VS;
	bd.Usage = BUFFER_CPU_WRITE_DISCARD;
	bd.TypeSize = sizeof(WVPBuffer);
	bd.NumElem = 1;
	bd.InitData = 0;
	this->mMatrixBuffer = new Buffer(device, bd);

	//Lightbuffer
	BUFFER_DESCRIPTION lbd;
	lbd.Type = CONSTANT_BUFFER_PS;
	lbd.Usage = BUFFER_CPU_WRITE_DISCARD;
	lbd.TypeSize = sizeof(LightBuffer);
	lbd.NumElem = 1;
	lbd.InitData = 0;

	this->mLightBuffer = new Buffer(device, lbd);
}

LightShaderClass::~LightShaderClass()
{
	delete this->mMatrixBuffer;
	delete this->mLightBuffer;
	delete mVPShader;
}

HRESULT LightShaderClass::Render(ID3D11DeviceContext* deviceContext, ID3D11SamplerState* samplerState, Camera* camera, int indexCount, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalTexture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	HRESULT hr = S_OK;
	
	//Use the color and normaltextures applied from the deferred vs/ps
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &normalTexture);
	
	//matrix buffer
	WVPBuffer* dataPtr = (WVPBuffer*)this->mMatrixBuffer->Map(deviceContext);

	dataPtr->World = DirectX::XMMatrixTranspose(XMMatrixIdentity());
	dataPtr->View = DirectX::XMMatrixTranspose(*camera->GetBaseViewMatrix());
	dataPtr->Projection = DirectX::XMMatrixTranspose(*camera->GetOrthoMatrix());

	this->mMatrixBuffer->Unmap(deviceContext);
	this->mMatrixBuffer->ApplyBuffer(deviceContext, 0, 0);

	//light buffer
	LightBuffer* dataPtr2 = (LightBuffer*)this->mLightBuffer->Map(deviceContext);
	
	dataPtr2->DiffuseColor = diffuseColor;
	dataPtr2->LightDirection = lightDirection;

	this->mLightBuffer->Unmap(deviceContext);
	this->mLightBuffer->ApplyBuffer(deviceContext, 0, 0);

	//Render Shader
	mVPShader->RenderIndexed(deviceContext, samplerState, indexCount);

	return hr;
}