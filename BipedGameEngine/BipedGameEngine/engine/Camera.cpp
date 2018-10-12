#include "Camera.h"
using namespace bpd;

Camera::Camera() {
	camPosition			= DirectX::XMVectorSet(0.0f,0.0f,0.0f,0.0f);
	camTarget			= DirectX::XMVectorSet(0.0f,0.0f,0.0f,0.0f);

	DefaultForward		= DirectX::XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	DefaultRight		= DirectX::XMVectorSet(1.0f,0.0f,0.0f,0.0f);

	camForward			= DirectX::XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	camRight			= DirectX::XMVectorSet(1.0f,0.0f,0.0f,0.0f);
	camUp				= DirectX::XMVectorSet(0.0f,1.0f,0.0f,0.0f);

	WVP					= DirectX::XMMATRIX();
	camView				= DirectX::XMMATRIX();
	camProjection		= DirectX::XMMATRIX();
	camRotationMatrix	= DirectX::XMMATRIX();

	moveLeftRight		= 0.0f;
	moveBackForward		= 0.0f;
	moveDownUp			= 0.0f;
	camYaw				= 0.0f;
	camPitch			= 0.0f;
}
Camera::~Camera() {}

void Camera::Update(){
	// Get the new rotation and update the camera target.
	camRotationMatrix	= DirectX::XMMatrixRotationRollPitchYaw(camPitch,camYaw,0);
	camTarget			= DirectX::XMVector3TransformCoord(DefaultForward,camRotationMatrix);
	camTarget			= DirectX::XMVector3Normalize(camTarget);

	// get the camera yaw and store it into a temp matrix.
	// We use this to update the local directions of the camera.
	DirectX::XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = DirectX::XMMatrixRotationY(camYaw);

	// Update the local camera directions with new camera rotation.
	camRight	= DirectX::XMVector3TransformCoord(DefaultRight,RotateYTempMatrix);
	camForward	= DirectX::XMVector3TransformCoord(DefaultForward,RotateYTempMatrix);
	camUp		= DirectX::XMVector3TransformCoord(camUp,RotateYTempMatrix);

	// store the vector in a float4 to edit the XYZW values within the vector
	DirectX::XMFLOAT4 tempPosition, tempTarget, tempRight;
	DirectX::XMStoreFloat4(&tempPosition, camPosition);
	DirectX::XMStoreFloat4(&tempTarget, camTarget);
	DirectX::XMStoreFloat4(&tempRight, camRight);

	// Add onto the position of the camera using the movement direction scaled by the movement amount.
	tempPosition.x += (moveLeftRight * tempRight.x) + (moveBackForward * tempTarget.x);
	tempPosition.y += moveDownUp + (moveBackForward * tempTarget.y);
	tempPosition.z += (moveLeftRight * tempRight.z) + (moveBackForward * tempTarget.z);

	// Load the temp float4 position back into the vector
	camPosition = DirectX::XMLoadFloat4(&tempPosition);

	// Update the camera target with the new camera position.
	tempTarget.x += tempPosition.x;
	tempTarget.y += tempPosition.y;
	tempTarget.z += tempPosition.z;
	tempTarget.w += tempPosition.w;

	// Load the temp float4 target back into the vector
	camTarget = DirectX::XMLoadFloat4(&tempTarget);

	// Set the camera view to the new camera position.
	camView = DirectX::XMMatrixLookAtLH(camPosition,camTarget,camUp);

	// make sure to set the movement back to 0
	// as not to continually move in that direction.
	moveLeftRight	= 0.0f;
	moveBackForward = 0.0f;
	moveDownUp		= 0.0f;
}