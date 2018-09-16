#pragma once
#include <vector>
#include <DXGI.h>
#include <D2D1.h>
#include <d3d11.h>
#include <sstream>
#include <Windows.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D2D1.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "D3D10_1.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "DirectXTK.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "Buffers.h"
#include "ErrorLogger.h"
#include "StringConverter.h"

#define SAFE_SHUTDOWN(x)	\
if (x != nullptr){			\
	x->Shutdown();			\
	delete x;				\
	x = NULL;				\
}
#define SAFE_DELETE(x)		\
if (x != nullptr){			\
	delete x;				\
	x = NULL;				\
}
#define SAFE_RELESE(x)		\
if (x != nullptr){			\
	x->Release();			\
	x = NULL;				\
}