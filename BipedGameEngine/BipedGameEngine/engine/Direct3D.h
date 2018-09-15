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

namespace bpd{
	class Direct3D {
	public:
		Direct3D();
		~Direct3D();

		bool Initialize(
			HINSTANCE hInstance,
			int width,
			int height,
			HWND hwnd
		);

		void Update(int deltaTime);
		void Render();
		void Shutdown();

		inline IDXGISwapChain		* GetSwapChain()		{ return SwapChain; }
		inline ID3D11Device			* GetDevice()			{ return d3d11Device; }
		inline ID3D11DeviceContext	* GetDeviceContext()	{ return d3d11DevCon; }

		HRESULT CleanupRenderTarget();
		HRESULT CreateRenderTarget();

	private:
		// SwapChain and Devices
		IDXGISwapChain			* SwapChain;
		ID3D11Device			* d3d11Device;
		ID3D11DeviceContext		* d3d11DevCon;

		// Render and Depth buffers
		ID3D11RenderTargetView	* renderTargetView;
		ID3D11DepthStencilView	* depthStencilView;
		ID3D11Texture2D			* depthStencilBuffer;

		// Vertex and Pixel shaders
		ID3D11VertexShader		* VS;
		ID3D11PixelShader		* PS;
		ID3D10Blob				* VS_Buffer;
		ID3D10Blob				* PS_Buffer;

		// Rasterizer States
		ID3D11RasterizerState	* CCWcullMode;
		ID3D11RasterizerState	* CWcullMode;
		ID3D11RasterizerState	* RSCullNone;

		// Blend and Sampler States
		ID3D11SamplerState		* LinearSamplerState;
		ID3D11BlendState		* Transparency;
		
		// Constant Buffers
		ID3D11Buffer			* cbPerObjectBuffer;
		ID3D11Buffer			* cbPerFrameBuffer;
		
		// BackBuffer
		ID3D11Texture2D			* BackBuffer11;
		
		// Layout
		ID3D11InputLayout		* vertLayout;
	};
}