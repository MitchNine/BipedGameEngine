#pragma once
#include "core/pch.h"

namespace bpd{
	class Camera {
	public:
		Camera();
		~Camera();

		void Update();

		DirectX::XMMATRIX WVP;
		DirectX::XMMATRIX camView;
		DirectX::XMMATRIX camProjection;
		DirectX::XMMATRIX camRotationMatrix;

		float moveLeftRight;
		float moveBackForward;
		float moveDownUp;
		float camYaw;
		float camPitch;

		DirectX::XMVECTOR camPosition;
		DirectX::XMVECTOR camTarget;
		DirectX::XMVECTOR camUp;

	private:
		DirectX::XMVECTOR DefaultForward;
		DirectX::XMVECTOR DefaultRight;
		DirectX::XMVECTOR camForward;
		DirectX::XMVECTOR camRight;
	};
}
