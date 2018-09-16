#pragma once
#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;

namespace bpd{
	struct Vertex {
		XMFLOAT3 pos;		// -- 12 -- 12
		XMFLOAT2 texCoord;	// -- 12 -- 24
		XMFLOAT3 normal;	// -- 12 -- 36
		XMFLOAT3 tangent;	// -- 12 -- 48
		XMFLOAT3 biTangent;	// -- 12 -- 60
		// 60 bytes
	};

	struct Light {
		Light() { ZeroMemory(this,sizeof(Light)); }
		XMFLOAT3 pos;		// -- 12 -- 12
		float range;		// -- 4  -- 16
		XMFLOAT3 dir;		// -- 12 -- 28
		float cone;			// -- 4  -- 32
		XMFLOAT3 att;		// -- 12 -- 44
		float pad2;			// -- 4  -- 48
		XMFLOAT4 ambient;	// -- 16 -- 64
		XMFLOAT4 diffuse;	// -- 16 -- 80
		// 80 bytes
	};


	// use BOOL as its 4 bytes instead of 1
	struct cbPerObject {
		XMMATRIX WVP	= XMMATRIX();	// -- 64 -- 0
		XMMATRIX World	= XMMATRIX();	// -- 64 -- 128
		XMFLOAT4 difColor;				// -- 16 -- 144
		BOOL hasTexture;				// -- 4  -- 148
		BOOL hasNormMap;				// -- 4  -- 152
		// 152 bytes
	};

	struct cbPerFrame {
		Light  light;	// -- 80 -- 80
		// 80 bytes
	};

	
}