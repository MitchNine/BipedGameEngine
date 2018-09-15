#pragma once
#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;

// use BOOL as its 4 bytes instead of 1
struct cbPerObject {
	XMMATRIX WVP	= XMMATRIX();	// -- 64 -- 0
	XMMATRIX World	= XMMATRIX();	// -- 64 -- 128
	XMFLOAT4 difColor;				// -- 16 -- 144
	BOOL hasTexture;				// -- 4 -- 148
	BOOL hasNormMap;				// -- 4 -- 152
	// 152 bytes
};

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
};