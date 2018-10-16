#pragma once
#include <Windows.h>
#include <DirectXMath.h>

namespace bpd{
	struct Vertex {
		DirectX::XMFLOAT3 pos;		// -- 12 -- 12
		DirectX::XMFLOAT2 texCoord;	// -- 8  -- 20
		DirectX::XMFLOAT3 normal;	// -- 12 -- 32
		DirectX::XMFLOAT3 tangent;	// -- 12 -- 44
		DirectX::XMFLOAT3 biTangent;// -- 12 -- 56
		// 56 bytes
	};

	struct Light {
		Light() { ZeroMemory(this,sizeof(Light)); }
		DirectX::XMFLOAT3 pos;		// -- 12 -- 12
		float range;				// -- 4  -- 16
		DirectX::XMFLOAT3 dir;		// -- 12 -- 28
		float cone;					// -- 4  -- 32
		DirectX::XMFLOAT3 att;		// -- 12 -- 44
		float pad2;					// -- 4  -- 48
		DirectX::XMFLOAT4 ambient;	// -- 16 -- 64
		DirectX::XMFLOAT4 diffuse;	// -- 16 -- 80
		// 80 bytes
	};

	// use BOOL as its 4 bytes instead of 1
	struct cbPerObject {
		DirectX::XMMATRIX WVP	= DirectX::XMMATRIX();	// -- 64 -- 64
		DirectX::XMMATRIX World	= DirectX::XMMATRIX();	// -- 64 -- 128
		DirectX::XMFLOAT4 difColor;						// -- 16 -- 144
		BOOL hasTexture;								// -- 4  -- 148
		BOOL hasNormMap;								// -- 4  -- 152
		// 152 bytes
	};

	struct cbPerObject_gizmos {
		DirectX::XMMATRIX WVP = DirectX::XMMATRIX();	// -- 64 -- 64
		DirectX::XMMATRIX World = DirectX::XMMATRIX();	// -- 64 -- 128
		DirectX::XMFLOAT4 color;						// -- 16 -- 144
		// 144 bytes
	};

	struct cbPerFrame {
		Light  light;	// -- 48 -- 48
		// 48 bytes
	};

	
}