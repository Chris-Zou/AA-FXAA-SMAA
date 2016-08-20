#include "ObjObject.h"

ObjObject::ObjObject(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	HRESULT hr = S_OK;
	this->mDevice = _device;
	this->mDeviceContext = _deviceContext;

	this->mVertexBuffer = nullptr;
	this->mIndexBuffer = nullptr;
	this->mLightBuffer = nullptr;

	this->mTexture = nullptr;
	this->mSamplerState = nullptr;

	this->mNrOfVertex = 0;
}

ObjObject::~ObjObject()
{
	SAFE_RELEASE(this->mVertexBuffer);
	SAFE_RELEASE(this->mIndexBuffer);
	SAFE_RELEASE(this->mLightBuffer);

	SAFE_RELEASE(this->mTexture);
	SAFE_RELEASE(this->mSamplerState);
}

HRESULT ObjObject::LoadFile(std::string _filename)
{
	HRESULT hr = S_OK;

	std::string part, mtllib, name;
	Vec2 dataVec2;
	Vec3 dataVec3;

	std::vector<Vec3>	v, vn;
	std::vector<Vec2>	vt;
	std::vector<int>	f;
	std::vector<Vertex>	vertexBufferData;
	std::vector<Group>	groups;
	std::vector<MTL>	mtls;

	std::ifstream objFile;
	_filename = "Assets/Obj/"+_filename;
	objFile.open(_filename);
	if(objFile.is_open())
	{
		while(!objFile.eof())
		{
			objFile >> part;

			if(part=="mtllib")
			{
				objFile >> mtllib;
			}
			if(part=="g")
			{
				objFile >> name;
				if(name!="default")
				{
					Group temp;
					temp.groupName=name;
					objFile >> name;
					objFile >> name;
					temp.useMtl=name;
					groups.push_back(temp);
				}
			}
			if(part=="v")
			{
				objFile >> dataVec3.x;
				objFile >> dataVec3.y;
				objFile >> dataVec3.z;
				v.push_back(dataVec3);				
			}
			if(part=="vt")
			{
				objFile >> dataVec2.x;
				objFile >> dataVec2.y;
				vt.push_back(dataVec2);
			}
			if(part=="vn")
			{
				objFile >> dataVec3.x;
				objFile >> dataVec3.y;
				objFile >> dataVec3.z;
				vn.push_back(dataVec3);
			}
			if(part=="f")
			{
				std::string v="";
				std::string vt="";
				std::string vn="";
				std::string line="";
				std::string::size_type sz;
				for(int i=0; i<3; i++)
				{
					objFile >> line;
					std::size_t pos=line.find('/');
					v=line.substr(0, pos);
					std::size_t secondPos=pos;
					
					line[pos]='.';

					pos=line.find('/');
					vt=line.substr(secondPos+1, pos-1);
					if(vt.find('/')!=std::string::npos)
					{
						vt.erase(vt.find('/'),vt.find('/'));
					}
					
					vn=line.substr(pos+1);
					
					f.push_back(std::stoi(v,&sz));
					f.push_back(std::stoi(vt,&sz));
					f.push_back(std::stoi(vn,&sz));
				}
			}
			part="";
		}
		OutputDebugString(L"Load from OBJ-file complete.\n");
		objFile.close();
		
		Vertex vertexData;
		if (vn.empty())
		{
			for (size_t i = 0; i < f.size(); i += 3)
			{
				vertexData.position = v.at(f.at(i) - 1);
				vertexData.texCoord = vt.at(f.at(i + 1) - 1);
				vertexBufferData.push_back(vertexData);
			}
		}
		else
		{
			for (size_t i = 0; i < f.size(); i += 3)
			{
				vertexData.position = v.at(f.at(i) - 1);
				vertexData.normal = vn.at(f.at(i + 2) - 1);
				vertexData.texCoord = vt.at(f.at(i + 1) - 1);
				vertexBufferData.push_back(vertexData);
			}
		}
		this->mNrOfVertex=vertexBufferData.size();

		hr = CreateBuffers(vertexBufferData);
		if(FAILED(hr))
		{
			OutputDebugString(L"Failed to create OBJ-buffers.\n");
			return hr;
		}
		hr = CreateMTL(mtllib, groups, mtls);
		if(FAILED(hr))
		{
			OutputDebugString(L"Failed to create MTL.\n");
			return hr;
		}
		hr = CreateTexture(mtls);
		if(FAILED(hr))
		{
			OutputDebugString(L"Failed to create OBJ texture.\n");
			return hr;
		}
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

void ObjObject::Render()
{
	this->mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT32 vertexSize = sizeof(Vertex);
	UINT32 offset = 0;
	ID3D11Buffer* buffersToSet[] = { this->mVertexBuffer };
	this->mDeviceContext->IASetVertexBuffers(0, 1, buffersToSet, &vertexSize, &offset);
	//this->mDeviceContext->PSSetShaderResources(0, 1, &this->mTexture);
	//this->mDeviceContext->Draw(this->mNrOfVertex, 0);
}

void ObjObject::SetObject(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale)
{
	mPosition = position;
	mRotation = rotation;
	mScale = scale;

	DirectX::XMMATRIX modelSpace = DirectX::XMMatrixIdentity();
	//Scale
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	modelSpace = DirectX::XMMatrixMultiply(modelSpace, S);
	//Rotation
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	//Position
	DirectX::XMMATRIX P = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	//Combining into localSpace
	DirectX::XMMATRIX tempSpace = XMMatrixMultiply(S, R);
	mModelSpace = XMMatrixMultiply(tempSpace, P);
}

int ObjObject::GetNumVertex() const
{
	return mNrOfVertex;
}

ID3D11ShaderResourceView * ObjObject::GetTexture()
{
	return mTexture;
}

DirectX::XMMATRIX * ObjObject::GetModelSpace()
{
	return &mModelSpace;
}

void ObjObject::Rotate(DirectX::XMFLOAT3 rotDelta)
{
	mRotation.x += rotDelta.x;
	mRotation.y += rotDelta.y;
	mRotation.z += rotDelta.z;
	SetObject(mPosition, mRotation, mScale);
}

HRESULT ObjObject::CreateBuffers(std::vector<Vertex> &_vertexBufferData)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC vertexDesc;
	memset(&vertexDesc, 0, sizeof(vertexDesc));
	vertexDesc.ByteWidth=sizeof(Vertex)*this->mNrOfVertex;
	vertexDesc.StructureByteStride=sizeof(Vertex);
	vertexDesc.Usage=D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags=D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.CPUAccessFlags	= 0;
	vertexDesc.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.SysMemPitch=0;
	vertexInitData.SysMemSlicePitch=0;
	vertexInitData.pSysMem=&_vertexBufferData[0];

	if(hr = FAILED(this->mDevice->CreateBuffer(&vertexDesc, &vertexInitData, &this->mVertexBuffer)))
	{
		OutputDebugString(L"Failed to create OBJ vertexbuffer.\n");
		return hr;
	}

	return hr;
}

HRESULT ObjObject::CreateMTL(std::string _mtllib, std::vector<Group> _groups, std::vector<MTL> &_mtls)
{
	HRESULT hr = S_OK;
	std::ifstream MtlFile;
	_mtllib = "Assets/Obj/"+_mtllib;
	MtlFile.open(_mtllib);
	std::string part, tempString;
	float tempvalue=0;
	Vec3 tempVec;
	MTL MData;
	if(MtlFile.is_open())
	{
		for(size_t i=0; i<_groups.size(); i++)
		{
			MtlFile >> part;
			if(part=="newmtl")
			{
				//newmtl material name
				MtlFile >> tempString;
				MData.newmtl=tempString;
				//illum
				MtlFile >> tempString >> tempvalue;
				MData.illum=tempvalue;
				//kd
				MtlFile >> tempString>> tempVec.x >> tempVec.y >> tempVec.z;
				MData.Kd=tempVec;
				//ka
				MtlFile >> tempString>> tempVec.x >> tempVec.y >> tempVec.z;
				MData.Ka=tempVec;
				//tf
				MtlFile >> tempString>> tempVec.x >> tempVec.y >> tempVec.z;
				MData.Tf=tempVec;
				//texturename
				MtlFile >> tempString>> tempString;
				MData.textureMap=tempString;
				//ni
				MtlFile >> tempString >> tempvalue;
				MData.Ni=tempvalue;
				_mtls.push_back(MData);
				part="";
			}
		}
		MtlFile.close();
	}
	else
	{
		return E_FAIL;
	}
	return hr;
}

HRESULT ObjObject::CreateTexture(std::vector<MTL> &_mtls)
{
	HRESULT hr = S_OK;

	std::wstring textureMap=std::wstring(_mtls.front().textureMap.begin(),_mtls.front().textureMap.end());
	textureMap = L"Assets/Obj/"+textureMap;
	const std::wstring s( textureMap.begin(), textureMap.end() );
	hr = D3DX11CreateShaderResourceViewFromFile(this->mDevice, s.c_str(), nullptr, nullptr, &this->mTexture, nullptr);
	if (FAILED(hr))
		throw std::runtime_error("D3DX11CreateShaderResourceViewFromFile");

	return hr;
}