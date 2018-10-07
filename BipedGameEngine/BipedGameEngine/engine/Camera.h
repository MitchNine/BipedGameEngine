#pragma once
#include "core/pch.h"

namespace bpd{
	class Camera {
	public:
		Camera();
		~Camera();

		void Update();

		XMMATRIX WVP;
		XMMATRIX camView;
		XMMATRIX camProjection;
		XMMATRIX camRotationMatrix;

		float moveLeftRight;
		float moveBackForward;
		float moveDownUp;
		float camYaw;
		float camPitch;

		XMVECTOR camPosition;
		XMVECTOR camTarget;
		XMVECTOR camUp;

	private:
		XMVECTOR DefaultForward;
		XMVECTOR DefaultRight;
		XMVECTOR camForward;
		XMVECTOR camRight;
	};
}
