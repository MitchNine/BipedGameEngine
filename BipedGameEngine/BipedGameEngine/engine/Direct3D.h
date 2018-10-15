#pragma once
#include "core/pch.h"

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
		bool InitializeBuffers();

		void Update(double deltaTime);
		void ClearScreen(float bgColor[4]);
		void Present();
		void Shutdown();

		inline IDXGISwapChain		* GetSwapChain()			{ return SwapChain; }
		inline ID3D11Device			* GetDevice()				{ return d3d11Device; }
		inline ID3D11DeviceContext	* GetDeviceContext()		{ return d3d11DevCon; }
		inline ID3D11Buffer			* GetCBPerObjectBuffer()	{ return cbPerObjectBuffer; }
		inline ID3D11Buffer			* GetCBPerFrameBuffer()		{ return cbPerFrameBuffer; }

		inline bpd::cbPerObject		GetCBPerObject()			{ return cbPerObj; }
		inline bpd::cbPerFrame		GetCBPerFrame()				{ return constbuffPerFrame; }

		inline bpd::Light			GETLight()					{ return light; }

		HRESULT CleanupRenderTarget();
		HRESULT CreateRenderTarget();

	private:
		bool CreateViewPort();
		bool CreateCBuffer();
		bool CreateSampleState();

	public:
		bool wireframe;
		bool fullscreen;
		bool vsync;

	private:
		int width;
		int height;

		ID3D11Debug				* pDebug;

		// hResult
		HRESULT					result;

		// SwapChain and Devices
		IDXGISwapChain			* SwapChain;
		ID3D11Device			* d3d11Device;
		ID3D11DeviceContext		* d3d11DevCon;

		// Render and Depth buffers
		ID3D11RenderTargetView	* renderTargetView;
		ID3D11DepthStencilView	* depthStencilView;
		ID3D11DepthStencilState * depthStencilState;
		ID3D11Texture2D			* depthStencilBuffer;

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
		ID3D11Texture2D			* BackBuffer;

		bpd::Light light;
		bpd::cbPerObject cbPerObj;
		bpd::cbPerFrame constbuffPerFrame;
	};
}