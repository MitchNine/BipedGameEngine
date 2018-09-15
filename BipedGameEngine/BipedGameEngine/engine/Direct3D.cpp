#include "Direct3D.h"
using namespace bpd;

Direct3D::Direct3D() {
	// SwapChain and Devices
	SwapChain			= 0;
	d3d11Device			= 0;
	d3d11DevCon			= 0;

	// Render and Depth buffers
	renderTargetView	= 0;
	depthStencilView	= 0;
	depthStencilBuffer	= 0;

	// Vertex and Pixel shaders
	VS					= 0;
	PS					= 0;
	VS_Buffer			= 0;
	PS_Buffer			= 0;

	// Rasterizer States
	CCWcullMode			= 0;
	CWcullMode			= 0;
	RSCullNone			= 0;

	// Blend and Sampler States
	LinearSamplerState	= 0;
	Transparency		= 0;

	// Constant Buffers
	cbPerObjectBuffer	= 0;
	cbPerFrameBuffer	= 0;

	// BackBuffer
	BackBuffer11		= 0;

	// Layout
	vertLayout			= 0;
}
Direct3D::~Direct3D() {}

bool Direct3D::Initialize(
	HINSTANCE hInstance,
	int width,
	int height,
	HWND hwnd
){
	HRESULT hr;

	// Describe our SwapChain Buffer
	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc,sizeof(DXGI_MODE_DESC));
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc,sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Create our Direct3D 11 Device and SwapChain
	hr = D3D11CreateDeviceAndSwapChain(0,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL,NULL,D3D11_SDK_VERSION,&swapChainDesc,&SwapChain,&d3d11Device,NULL,&d3d11DevCon);
	if (FAILED(hr)) return false;

	// Create our BackBuffer and Render Target
	hr = CreateRenderTarget();
	if(FAILED(hr)) return false;

	// Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// Create the Depth/Stencil View
	hr = d3d11Device->CreateTexture2D(&depthStencilDesc,NULL,&depthStencilBuffer);
	if(FAILED(hr)) return false;

	hr = d3d11Device->CreateDepthStencilView(depthStencilBuffer,NULL,&depthStencilView);
	if(FAILED(hr)) return false;

	return true;
}

void Direct3D::Shutdown(){
	// Exit fullscreen
	SwapChain->SetFullscreenState(false,NULL);

	// SwapChain and Devices
	SAFE_RELESE(SwapChain);
	SAFE_RELESE(d3d11Device);
	SAFE_RELESE(d3d11DevCon);

	// Render and Depth buffers
	SAFE_RELESE(renderTargetView);
	SAFE_RELESE(depthStencilView);
	SAFE_RELESE(depthStencilBuffer);

	// Vertex and Pixel shaders
	SAFE_RELESE(VS);
	SAFE_RELESE(PS);
	SAFE_RELESE(VS_Buffer);
	SAFE_RELESE(PS_Buffer);

	// Rasterizer States
	SAFE_RELESE(CCWcullMode);
	SAFE_RELESE(CWcullMode);
	SAFE_RELESE(RSCullNone);

	// Blend and Sampler States
	SAFE_RELESE(LinearSamplerState);
	SAFE_RELESE(Transparency);

	// Constant Buffers
	SAFE_RELESE(cbPerObjectBuffer);
	SAFE_RELESE(cbPerFrameBuffer);

	// BackBuffer
	SAFE_RELESE(BackBuffer11);

	// Layout
	SAFE_RELESE(vertLayout);
}

void Direct3D::Update(int deltaTime){
	
}

void Direct3D::Render(){
	
}

HRESULT Direct3D::CleanupRenderTarget(){
	if (renderTargetView == nullptr) return S_FALSE;
	SAFE_RELESE(renderTargetView);
	return S_OK;
}

HRESULT Direct3D::CreateRenderTarget(){
	HRESULT hr;

	hr = SwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&BackBuffer11);
	if (FAILED(hr)) return hr;

	hr = d3d11Device->CreateRenderTargetView(BackBuffer11,NULL,&renderTargetView);
	if(FAILED(hr)) return hr;

	return hr;
}