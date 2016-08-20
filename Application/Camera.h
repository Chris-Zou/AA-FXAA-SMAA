#pragma once

#include <DirectXMath.h>

enum CamMode
{
	FREE_CAM,
	LOCKED_CAM
};

class Camera
{
private:
	DirectX::XMFLOAT3	_position;
	DirectX::XMFLOAT3	_right;
	DirectX::XMFLOAT3	_rotatedRight;
	DirectX::XMFLOAT3	_up;
	DirectX::XMFLOAT3	_forward;
	DirectX::XMFLOAT3	_rotatedForward;
	DirectX::XMFLOAT3	_rotation;

	float				_nearClip;
	float				_farClip;
	float				_fieldOfView;
	float				_aspectRatio;

	DirectX::XMMATRIX	_view;
	DirectX::XMMATRIX	_proj;
	DirectX::XMMATRIX	_ortho;
	DirectX::XMMATRIX	_baseView;

	CamMode				_mode;

public:
	Camera();
	Camera(float nearClip, float farClip, float fov, DirectX::XMINT2 screenSize);
	~Camera();
	void Initialize(float nearClip, float farClip, float fov, DirectX::XMINT2 screenSize);

	void Update();
	void Resize(int width, int height);

	DirectX::XMFLOAT3 GetPosition()const;
	void SetPosition(DirectX::XMFLOAT3 position);
	void Move(DirectX::XMFLOAT3 offset);
	DirectX::XMFLOAT3 GetRotation()const;
	void SetRotation(DirectX::XMFLOAT3 rotation);

	DirectX::XMMATRIX* GetViewMatrix();
	DirectX::XMMATRIX* GetProjectionMatrix();
	//OrthoMatrix is commonly used for anything 2D. Commonly used in the "projection slot".
	DirectX::XMMATRIX* GetOrthoMatrix();
	//BaseViewMatrix is used for 2D HUD. Commonly used in the "view slot".
	DirectX::XMMATRIX* GetBaseViewMatrix();

	DirectX::XMFLOAT3 GetForwardVector() const;
	DirectX::XMFLOAT3 GetRightVector() const;

	CamMode GetMode()const;
	void SetMode(CamMode mode);

	//Overloading these guarantees 16B alignment of XMMATRIX
	void* operator new(size_t i);
	void operator delete(void* p);

};