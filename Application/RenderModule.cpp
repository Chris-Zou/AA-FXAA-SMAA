#include "RenderModule.h"
#include <string>

RenderModule::RenderModule(DirectXClass* directX, DirectX::XMINT2 screenSize)
{
	HRESULT hr = S_OK;

	_directX = directX;
	_device = _directX->GetDevice();
	_deviceContext = _directX->GetDeviceContext();
	
	//WVP Buffer
	BUFFER_DESCRIPTION bd;
	bd.Type = CONSTANT_BUFFER_VS;
	bd.Usage = BUFFER_CPU_WRITE_DISCARD;
	bd.TypeSize = sizeof(WVPBuffer);
	bd.NumElem = 1;
	bd.InitData = 0;
	_matrixBuffer = new Buffer(_device, bd);

	CreateShaders(_device);
	_AAHandler = new AntiAliasingHandler(_device, _directX, screenSize, _directX->GetBackBufferRTV(), _directX->GetBackBufferDSV());

	_orthoWindow = new OrthoWindowClass(_device, screenSize);
	_samplerState = directX->GetSamplerState();

	_light.SetDiffuseColor(0.1f, 0.1f, 0.1f, 1.0f);
	_light.SetDirection(0.0f, 0.0f, 1.0f);
}

RenderModule::~RenderModule()
{
	delete _vpShader;
	delete _lightShader;
	delete _orthoWindow;
	delete _AAHandler;
	delete _matrixBuffer;
}

void RenderModule::Update(Camera* camera)
{
	//matrix buffer
	WVPBuffer* dataPtr = (WVPBuffer*)this->_matrixBuffer->Map(_deviceContext);

	DirectX::XMMATRIX id = DirectX::XMMatrixIdentity();

	dataPtr->World = DirectX::XMMatrixTranspose(id);
	dataPtr->View = DirectX::XMMatrixTranspose(*camera->GetViewMatrix());
	dataPtr->Projection = DirectX::XMMatrixTranspose(*camera->GetProjectionMatrix());

	this->_matrixBuffer->Unmap(_deviceContext);
	this->_matrixBuffer->ApplyBuffer(_deviceContext, 0, 0);
}

void RenderModule::RenderVertex(Camera* camera, ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState, DirectX::XMMATRIX* localspace, int numVertex)
{
	_deviceContext->PSSetShaderResources(0, 1, &texture);

	//matrix buffer
	WVPBuffer* dataPtr = (WVPBuffer*)this->_matrixBuffer->Map(_deviceContext);

	dataPtr->World = DirectX::XMMatrixTranspose(DirectX::XMMATRIX(*localspace));
	dataPtr->View = DirectX::XMMatrixTranspose(*camera->GetViewMatrix());
	dataPtr->Projection = DirectX::XMMatrixTranspose(*camera->GetProjectionMatrix());

	this->_matrixBuffer->Unmap(_deviceContext);
	this->_matrixBuffer->ApplyBuffer(_deviceContext, 0, 0);

	// Render
	this->_vpShader->RenderVertex(_deviceContext, samplerState, numVertex);
}

void RenderModule::RenderIndexed(Camera* camera, ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState, DirectX::XMMATRIX* localspace, int numIndex)
{
	_deviceContext->PSSetShaderResources(0, 1, &texture);

	//matrix buffer
	WVPBuffer* dataPtr = (WVPBuffer*)this->_matrixBuffer->Map(_deviceContext);

	dataPtr->World = DirectX::XMMatrixTranspose(DirectX::XMMATRIX(*localspace));
	dataPtr->View = DirectX::XMMatrixTranspose(*camera->GetViewMatrix());
	dataPtr->Projection = DirectX::XMMatrixTranspose(*camera->GetProjectionMatrix());

	this->_matrixBuffer->Unmap(_deviceContext);
	this->_matrixBuffer->ApplyBuffer(_deviceContext, 0, 0);

	// Render
	this->_vpShader->RenderIndexed(_deviceContext, samplerState, numIndex);
}

void RenderModule::RenderLight(Camera* camera, int aliasingMethod)
{
	//turn off zbuffer to render onto the ortho
	_directX->TurnZBufferOff();
	//_directX->ClearRender();
	//Apply the ortho window buffers
	_orthoWindow->Render(_deviceContext);

	//Fix rendertargets
	if (aliasingMethod == AntiAliasingMethod::FXAA)
	{
		_directX->SetLightRenderTarget(false);
		//Use the vs/ps to render to the buffers.
		_lightShader->Render(_deviceContext, _samplerState, camera, _orthoWindow->GetIndexCount(), _directX->GetShaderResourceView(0, false), _directX->GetShaderResourceView(1, false), _light.GetDirection(), _light.GetDiffuseColor());
	}
	else
	{
		_directX->SetLightRenderTarget(true);
		//Use the vs/ps to render to the buffers.
		_lightShader->Render(_deviceContext, _samplerState, camera, _orthoWindow->GetIndexCount(), _directX->GetShaderResourceView(0, true), _directX->GetShaderResourceView(1, true), _light.GetDirection(), _light.GetDiffuseColor());
	}
	//turn on zbuffer again
	_directX->TurnZBufferOn();
}

void RenderModule::RenderAntiAliasing(int method)
{
	_directX->TurnZBufferOff();
	_orthoWindow->Render(_deviceContext);
	if(method == AntiAliasingMethod::FXAA)
		_AAHandler->Render(method, _deviceContext, _directX->GetShaderResourceView(2, false), _orthoWindow->GetIndexCount(), _directX->GetSamplerState(), _orthoWindow->GetScreenSize());
	else
		_AAHandler->Render(method, _deviceContext, _directX->GetShaderResourceView(2, true), _orthoWindow->GetIndexCount(), _directX->GetSamplerState(), _orthoWindow->GetScreenSize());
	_directX->TurnZBufferOn();
}

HRESULT RenderModule::CreateShaders(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	_lightShader = new LightShaderClass(device);
	
	//Normal Vertex Pixel Shader
	SHADER_DESCRIPTION sd;
	sd.vertexFilePath = "Assets/Shaders/DeferredVertexShader.hlsl";
	sd.vertexHeader = "VS_Deferred";
	sd.vertexNumElem = 3;
	sd.inputDesc = new D3D11_INPUT_ELEMENT_DESC[3];

	sd.inputDesc[0].SemanticName = "POSITION";
	sd.inputDesc[0].SemanticIndex = 0;
	sd.inputDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	sd.inputDesc[0].InputSlot = 0;
	sd.inputDesc[0].AlignedByteOffset = 0;
	sd.inputDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[0].InstanceDataStepRate = 0;

	sd.inputDesc[1].SemanticName = "NORMAL";
	sd.inputDesc[1].SemanticIndex = 0;
	sd.inputDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	sd.inputDesc[1].InputSlot = 0;
	sd.inputDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	sd.inputDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[1].InstanceDataStepRate = 0;

	sd.inputDesc[2].SemanticName = "TEXCOORD";
	sd.inputDesc[2].SemanticIndex = 0;
	sd.inputDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	sd.inputDesc[2].InputSlot = 0;
	sd.inputDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	sd.inputDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	sd.inputDesc[2].InstanceDataStepRate = 0;

	sd.pixelFilePath = "Assets/Shaders/DeferredPixelShader.hlsl";
	sd.pixelHeader = "PS_Deferred";

	_vpShader = new Shader(device, sd);

	delete[]sd.inputDesc;
	return hr;
}