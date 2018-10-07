#include "Camera.h"
using namespace bpd;

Camera::Camera() {
	DefaultForward = DirectX::XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	DefaultRight = DirectX::XMVectorSet(1.0f,0.0f,0.0f,0.0f);

	camForward = DirectX::XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	camRight = DirectX::XMVectorSet(1.0f,0.0f,0.0f,0.0f);

	WVP = XMMATRIX();
	camView = XMMATRIX();
	camProjection = XMMATRIX();
	camRotationMatrix = XMMATRIX();

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	moveDownUp = 0.0f;
	camYaw = 0.0f;
	camPitch = 0.0f;

	camPosition = XMVECTOR();
	camTarget = XMVECTOR();
	camUp = XMVECTOR();
}


Camera::~Camera() {

}

void Camera::Update(){
	camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(camPitch,camYaw,0);
	camTarget = DirectX::XMVector3TransformCoord(DefaultForward,camRotationMatrix);
	camTarget = DirectX::XMVector3Normalize(camTarget);

	DirectX::XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = DirectX::XMMatrixRotationY(camYaw);

	camRight = DirectX::XMVector3TransformCoord(DefaultRight,RotateYTempMatrix);
	camForward = DirectX::XMVector3TransformCoord(DefaultForward,RotateYTempMatrix);
	camUp = DirectX::XMVector3TransformCoord(camUp,RotateYTempMatrix);

	if(moveLeftRight != 0) camPosition = DirectX::XMVectorAdd(camPosition,DirectX::XMVectorScale(camRight,moveLeftRight));
	if(moveBackForward != 0) camPosition = DirectX::XMVectorAdd(camPosition,DirectX::XMVectorScale(camTarget,moveBackForward));
	if(moveDownUp != 0) camPosition = DirectX::XMVectorAdd(camPosition,DirectX::XMVectorScale(camUp,moveDownUp));

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	moveDownUp = 0.0f;

	camTarget = DirectX::XMVectorAdd(camPosition,camTarget);

	camView = DirectX::XMMatrixLookAtLH(camPosition,camTarget,camUp);
}