#pragma once
#include "core/pch.h"

namespace bpd{
	class Direct3D {
	public:
		Direct3D();
		~Direct3D();

		enum RasterizerState {
			eCCWcullMode,
			eCWcullMode,
			eRSCullNone,
			eRSCullNoneWF,
		};

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

		inline bpd::cbPerObject		GetCBPerObject()			{ return cbPerObj; }
		inline bpd::cbPerFrame		GetCBPerFrame()				{ return constbuffPerFrame; }

		inline bpd::Light			GETLight()					{ return light; }

		void SetRasterizerState(RasterizerState state);

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

		// Constant Buffers
		ID3D11Buffer			* cbPerObjectBuffer;
		ID3D11Buffer			* cbPerObjectBuffer_gizmos;
		ID3D11Buffer			* cbPerFrameBuffer;

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
		ID3D11RasterizerState	* RSCullNoneWF;

		// Blend and Sampler States
		ID3D11SamplerState		* LinearSamplerState;
		ID3D11BlendState		* Transparency;
				
		// BackBuffer
		ID3D11Texture2D			* BackBuffer;

		bpd::Light light;
		bpd::cbPerObject cbPerObj;
		bpd::cbPerFrame constbuffPerFrame;
	};
}