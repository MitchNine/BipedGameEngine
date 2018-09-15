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

	// flags
	wireframe			= false;
	fullscreen			= false;
	vsync				= false;
}
Direct3D::~Direct3D() {}

bool Direct3D::Initialize(
	HINSTANCE hInstance,
	int width,
	int height,
	HWND hwnd
){
	// Describe our SwapChain Buffer
	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc,sizeof(DXGI_MODE_DESC));
	bufferDesc.Width					= width;
	bufferDesc.Height					= height;
	bufferDesc.RefreshRate.Numerator	= 60;
	bufferDesc.RefreshRate.Denominator	= 1;
	bufferDesc.Format					= DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;

	// Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc,sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc			= bufferDesc;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.SampleDesc.Quality	= 0;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount			= 1;
	swapChainDesc.OutputWindow			= hwnd;
	swapChainDesc.Windowed				= !fullscreen;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;

	// Create our Direct3D 11 Device and SwapChain
	result = D3D11CreateDeviceAndSwapChain(0,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL,NULL,D3D11_SDK_VERSION,&swapChainDesc,&SwapChain,&d3d11Device,NULL,&d3d11DevCon);
	if (FAILED(result)) return false;

	// Create our BackBuffer and Render Target
	result = CreateRenderTarget();
	if(FAILED(result)) return false;

	// Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width				= width;
	depthStencilDesc.Height				= height;
	depthStencilDesc.MipLevels			= 1;
	depthStencilDesc.ArraySize			= 1;
	depthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count	= 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage				= D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags		= 0;
	depthStencilDesc.MiscFlags			= 0;

	// Create the Depth/Stencil View
	result = d3d11Device->CreateTexture2D(&depthStencilDesc,NULL,&depthStencilBuffer);
	if(FAILED(result)) return false;

	result = d3d11Device->CreateDepthStencilView(depthStencilBuffer,NULL,&depthStencilView);
	if(FAILED(result)) return false;

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

void Direct3D::Update(double deltaTime){
	
}

void Direct3D::Render(){
	
}

HRESULT Direct3D::CleanupRenderTarget(){
	if (renderTargetView == nullptr) return S_FALSE;
	SAFE_RELESE(renderTargetView);
	return S_OK;
}

HRESULT Direct3D::CreateRenderTarget(){
	result = SwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&BackBuffer11);
	if (FAILED(result)) return result;

	result = d3d11Device->CreateRenderTargetView(BackBuffer11,NULL,&renderTargetView);
	if(FAILED(result)) return result;

	return result;
}

bool Direct3D::CreateViewPort(int width,int height){
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX	= 0;
	viewport.TopLeftY	= 0;
	viewport.Width		= (FLOAT)width;
	viewport.Height		= (FLOAT)height;
	viewport.MinDepth	= 0.0f;
	viewport.MaxDepth	= 1.0f;

	d3d11DevCon->RSSetViewports(1,&viewport);
	return true;
}

bool Direct3D::CreateCBuffer(){
	D3D11_BUFFER_DESC cbbd;

	// Per object buffer
	ZeroMemory(&cbbd,sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage			= D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth		= sizeof(bpd::cbPerObject);
	cbbd.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags		= 0;

	result = d3d11Device->CreateBuffer(&cbbd,NULL,&cbPerObjectBuffer);
	if (FAILED(result)) return false;

	// Per frame buffer
	ZeroMemory(&cbbd,sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage			= D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth		= sizeof(bpd::cbPerFrame);
	cbbd.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags		= 0;

	result = d3d11Device->CreateBuffer(&cbbd,NULL,&cbPerFrameBuffer);
	if(FAILED(result)) return false;

	return true;
}

bool Direct3D::CreateSampleState(){
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc,sizeof(sampDesc));
	sampDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD			= 0;
	sampDesc.MaxLOD			= D3D11_FLOAT32_MAX;

	//Create the Sample State
	result = d3d11Device->CreateSamplerState(&sampDesc,&LinearSamplerState);
	if(FAILED(result)) return false;


	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc,sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode					= D3D11_FILL_SOLID;
	cmdesc.CullMode					= D3D11_CULL_BACK;
	cmdesc.FrontCounterClockwise	= true;
	result = d3d11Device->CreateRasterizerState(&cmdesc,&CCWcullMode);
	if(FAILED(result)) return false;

	cmdesc.FrontCounterClockwise = false;

	result = d3d11Device->CreateRasterizerState(&cmdesc,&CWcullMode);
	if(FAILED(result)) return false;

	if (wireframe)
		cmdesc.FillMode = D3D11_FILL_WIREFRAME;
	cmdesc.CullMode		= D3D11_CULL_NONE;
	result = d3d11Device->CreateRasterizerState(&cmdesc,&RSCullNone);
	if(FAILED(result)) return false;

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc,sizeof(blendDesc));
	
	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd,sizeof(rtbd));

	rtbd.BlendEnable			= true;
	rtbd.SrcBlend				= D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.DestBlend				= D3D11_BLEND_SRC_ALPHA;
	rtbd.BlendOp				= D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha			= D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.DestBlendAlpha			= D3D11_BLEND_SRC_ALPHA;
	rtbd.BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask	= D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0]		= rtbd;

	d3d11Device->CreateBlendState(&blendDesc,&Transparency);

	return true;
}