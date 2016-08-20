#include "Camera.h"

using namespace DirectX;

Camera::Camera(){}

Camera::Camera(float nearClip, float farClip, float fov, DirectX::XMINT2 screenSize)
{
	_nearClip = nearClip; //0.5f;
	_farClip = farClip; // 1000.0f;
	_fieldOfView = fov;
	_aspectRatio = (float)screenSize.x / (float)screenSize.y;

	_position = DirectX::XMFLOAT3(0, 0, 0);
	_right = DirectX::XMFLOAT3(1, 0, 0);
	_up = DirectX::XMFLOAT3(0, 1, 0);
	_rotatedForward = DirectX::XMFLOAT3(0, 0, 1);
	_forward = DirectX::XMFLOAT3(0, 0, 1);
	_rotation = DirectX::XMFLOAT3(0, 0, 0);


	//Prepare vectors for Matrix initialization
	DirectX::XMVECTOR vPos, vFor, vUp;
	vPos = DirectX::XMLoadFloat3(&_position);
	vFor = DirectX::XMLoadFloat3(&_forward);
	vUp = DirectX::XMLoadFloat3(&_up);

	_view = DirectX::XMMatrixLookAtLH(vPos, vFor, vUp);
	_proj = DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _aspectRatio, _nearClip, _farClip);
	_ortho = DirectX::XMMatrixOrthographicLH((float)screenSize.x, (float)screenSize.y, _nearClip, _farClip);
	_baseView = DirectX::XMMatrixLookAtLH(DirectX::XMVectorNegate(vFor), vFor, vUp);

	_mode = LOCKED_CAM;
}

void Camera::Initialize(float nearClip, float farClip, float fov, XMINT2 screenSize)
{
	_nearClip = nearClip; //0.5f;
	_farClip = farClip; // 1000.0f;
	_fieldOfView = fov;
	_aspectRatio = (float)screenSize.x / (float)screenSize.y;

	_position = DirectX::XMFLOAT3(0, 0, 0);
	_right = DirectX::XMFLOAT3(1, 0, 0);
	_up = DirectX::XMFLOAT3(0, 1, 0);
	_rotatedForward = DirectX::XMFLOAT3(0, 0, 1);
	_forward = DirectX::XMFLOAT3(0, 0, 1);
	_rotation = DirectX::XMFLOAT3(0, 0, 0);


	//Prepare vectors for Matrix initialization
	DirectX::XMVECTOR vPos, vFor, vUp;
	vPos = DirectX::XMLoadFloat3(&_position);
	vFor = DirectX::XMLoadFloat3(&_forward);
	vUp = DirectX::XMLoadFloat3(&_up);

	_view = DirectX::XMMatrixLookAtLH(vPos, vFor, vUp);
	_proj = DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _aspectRatio, _nearClip, _farClip);
	_ortho = DirectX::XMMatrixOrthographicLH((float)screenSize.x, (float)screenSize.y, _nearClip, _farClip);
	_baseView = DirectX::XMMatrixLookAtLH(DirectX::XMVectorNegate(vFor), vFor, vUp);

	_mode = LOCKED_CAM;
}

Camera::~Camera()
{}

void Camera::Update()
{
	XMMATRIX rotationMatrix;

	XMVECTOR up = XMLoadFloat3(&_up);
	XMVECTOR pos = XMLoadFloat3(&_position);
	XMVECTOR forward = XMLoadFloat3(&_forward);
	XMVECTOR right = XMLoadFloat3(&_right);

	rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(_rotation.x), XMConvertToRadians(_rotation.y), 0);

	forward = XMVector3TransformCoord(forward, rotationMatrix);
	XMStoreFloat3(&_rotatedForward, forward);

	up = XMVector3TransformCoord(up, rotationMatrix);

	right = XMVector3TransformCoord(right, rotationMatrix);
	XMStoreFloat3(&_rotatedRight, right);

	_view = XMMatrixLookAtLH(pos, pos + forward, up);
}

void Camera::Resize(int width, int height)
{
	_aspectRatio = (float)width / (float)height;
	_proj = DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _aspectRatio, _nearClip, _farClip);
	_ortho = DirectX::XMMatrixOrthographicLH((float)width, (float)height, _nearClip, _farClip);
}

DirectX::XMFLOAT3 Camera::GetPosition()const
{
	return _position;
}

void Camera::SetPosition(DirectX::XMFLOAT3 position)
{
	_position = position;
	Update();
}

void Camera::Move(DirectX::XMFLOAT3 offset)
{
	_position.x += offset.x;
	_position.y += offset.y;
	_position.z += offset.z;
	Update();
}

DirectX::XMFLOAT3 Camera::GetRotation()const
{
	return _rotation;
}

void Camera::SetRotation(DirectX::XMFLOAT3 rotation)
{
	_rotation = rotation;
	Update();
}

DirectX::XMMATRIX* Camera::GetViewMatrix()
{
	return &_view;
}
DirectX::XMMATRIX* Camera::GetProjectionMatrix()
{
	return &_proj;
}
DirectX::XMMATRIX* Camera::GetOrthoMatrix()
{
	return &_ortho;
}
DirectX::XMMATRIX* Camera::GetBaseViewMatrix()
{
	return &_baseView;
}

DirectX::XMFLOAT3 Camera::GetForwardVector() const
{
	return _rotatedForward;
}
DirectX::XMFLOAT3 Camera::GetRightVector() const
{
	return _rotatedRight;
}

CamMode Camera::GetMode() const
{
	return _mode;
}
void Camera::SetMode(CamMode mode)
{
	_mode = mode;
}


void* Camera::operator new(size_t i)
{
	return _mm_malloc(i, 16);
}

void Camera::operator delete(void* p)
{
	_mm_free(p);
}