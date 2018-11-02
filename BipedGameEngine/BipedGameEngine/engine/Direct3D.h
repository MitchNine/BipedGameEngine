#pragma once
#include "core/pch.h"

namespace bpd{
	class Direct3D {
	public:
		Direct3D();
		~Direct3D();

		// Enum of rasterizer states to be used
		enum RasterizerState {
			eCCWcullMode,
			eCWcullMode,
			eRSCullNone,
			eRSCullNoneWF,
		};

		// Initialize DirectX 11
		// (Device, SwapChain, DepthStencil, ViewPort, Constant Buffers, SampleStates, light)
		bool Initialize(
			HINSTANCE hInstance,
			int width,
			int height,
			HWND hwnd
		);

		// Update anything that needs to be updated
		// NOT IN USE AT THE MOMENT
		void Update(double deltaTime);

		// Clear the scene and bind the buffers
		void ClearScreen(float bgColor[4]);

		// Present to the GPU
		void Present();

		// Clear all pointer
		void Shutdown();

		// Returns the SwapChain
		inline IDXGISwapChain		* GetSwapChain()			{ return SwapChain; }
		// Returns the Device
		inline ID3D11Device			* GetDevice()				{ return d3d11Device; }
		// Returns the DeviceContext
		inline ID3D11DeviceContext	* GetDeviceContext()		{ return d3d11DevCon; }

		// Returns the constant buffer (PerObject)
		inline bpd::cbPerObject		GetCBPerObject()			{ return cbPerObj; }
		// Returns the constant buffer (PerFrame)
		inline bpd::cbPerFrame		GetCBPerFrame()				{ return constbuffPerFrame; }

		// Returns the light
		inline bpd::Light			GETLight()					{ return light; }

		// Binds the Rasterizer State that will be used
		void SetRasterizerState(RasterizerState state);

		// Clean up the render target
		HRESULT CleanupRenderTarget();
		// Creates a new Render Target
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