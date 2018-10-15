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
	BackBuffer		= 0;

	// flags
	wireframe			= false;
	fullscreen			= false;
	vsync				= false;
}
Direct3D::~Direct3D() {
	
}

bool Direct3D::Initialize(
	HINSTANCE hInstance,
	int width,
	int height,
	HWND hwnd
){
	this->width		= width;
	this->height	= height;

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
	swapChainDesc.SampleDesc.Count		= 4;
	swapChainDesc.SampleDesc.Quality	= 4;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount			= 1;
	swapChainDesc.OutputWindow			= hwnd;
	swapChainDesc.Windowed				= !fullscreen;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	D3D_FEATURE_LEVEL featureD11_1 = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL featureD11_0 = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL featureD10_1 = D3D_FEATURE_LEVEL_10_1;
	D3D_FEATURE_LEVEL featureD10_0 = D3D_FEATURE_LEVEL_10_0;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// Create our Direct3D 11 Device and SwapChain
	if(FAILED(D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_DEBUG,featureLevels,numFeatureLevels,D3D11_SDK_VERSION,&swapChainDesc,&SwapChain,&d3d11Device,&featureD11_1,&d3d11DevCon)))
		if(FAILED(D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_DEBUG,featureLevels,numFeatureLevels,D3D11_SDK_VERSION,&swapChainDesc,&SwapChain,&d3d11Device,&featureD11_0,&d3d11DevCon)))
			if(FAILED(D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_DEBUG,featureLevels,numFeatureLevels,D3D11_SDK_VERSION,&swapChainDesc,&SwapChain,&d3d11Device,&featureD10_1,&d3d11DevCon)))
				if(FAILED(D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_DEBUG,featureLevels,numFeatureLevels,D3D11_SDK_VERSION,&swapChainDesc,&SwapChain,&d3d11Device,&featureD10_0,&d3d11DevCon)))
					return false;

	if (FAILED(d3d11Device->QueryInterface(IID_PPV_ARGS(&pDebug))))
		return false;

	// Create our BackBuffer and Render Target
	if (FAILED(CreateRenderTarget()))
		return false;
	

	// Describe our Depth Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width				= this->width;
	depthStencilDesc.Height				= this->height;
	depthStencilDesc.MipLevels			= 1;
	depthStencilDesc.ArraySize			= 1;
	depthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count	= swapChainDesc.SampleDesc.Count;
	depthStencilDesc.SampleDesc.Quality = swapChainDesc.SampleDesc.Quality;
	depthStencilDesc.Usage				= D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags		= 0;
	depthStencilDesc.MiscFlags			= 0;

	// Create the Depth/Stencil View
	if(FAILED( d3d11Device->CreateTexture2D(&depthStencilDesc,NULL,&depthStencilBuffer)))
		return false;

	// Describe our Depth Stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	if(FAILED(d3d11Device->CreateDepthStencilState(&dsDesc,&depthStencilState)))
		return false;

	// Describe our depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc,sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.Texture2DMS.UnusedField_NothingToDefine = 0;

	// Create depth stencil view
	if(FAILED(d3d11Device->CreateDepthStencilView(depthStencilBuffer,&depthStencilViewDesc,&depthStencilView)))
		return false;

	if(!CreateViewPort())		return false;
	if(!CreateCBuffer())		return false;
	if(!CreateSampleState())	return false;

	light.pos = DirectX::XMFLOAT3(0.0f,7.0f,0.0f);
	light.dir = DirectX::XMFLOAT3(0.5f,0.75f,-0.5f);
	light.range = 1000.0f;
	light.cone = 12.0f;
	light.att = DirectX::XMFLOAT3(0.4f,0.02f,0.000f);
	light.ambient = DirectX::XMFLOAT4(0.2f,0.2f,0.2f,1.0f);
	light.diffuse = DirectX::XMFLOAT4(1.0f,1.0f,1.0f,1.0f);

	return true;
}

bool Direct3D::InitializeBuffers(){
	if (!CreateViewPort())		return false;
	if (!CreateCBuffer())		return false;
	if (!CreateSampleState())	return false;
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
	SAFE_RELESE(depthStencilState);
	SAFE_RELESE(depthStencilBuffer);

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
	SAFE_RELESE(BackBuffer);

	if (pDebug != nullptr){
		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		pDebug = nullptr;
	}
}

void Direct3D::Update(double deltaTime){
	
}

void Direct3D::ClearScreen(float bgColor[4]){
	// Clear our render target and depth/stencil view
	d3d11DevCon->ClearRenderTargetView(renderTargetView,bgColor);
	d3d11DevCon->ClearDepthStencilView(depthStencilView,D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0f,0);

	// Update the cbPerFrame
	constbuffPerFrame.light = light;
	d3d11DevCon->UpdateSubresource(cbPerFrameBuffer,0,NULL,&constbuffPerFrame,0,0);
	d3d11DevCon->PSSetConstantBuffers(0,1,&cbPerFrameBuffer);

	// Set the depth stencil state
	d3d11DevCon->OMSetDepthStencilState(depthStencilState,1);

	// Set our Render Target
	d3d11DevCon->OMSetRenderTargets(1,&renderTargetView,depthStencilView);

	// Set the default blend state (no blending) for opaque objects
	d3d11DevCon->OMSetBlendState(0,0,0xffffffff);
	
	// Set sampler and rasterizer
	d3d11DevCon->PSSetSamplers(0,1,&LinearSamplerState);
	d3d11DevCon->RSSetState(RSCullNone);
}

void Direct3D::Present(){
	//Present the back buffer to the screen
	if (vsync) SwapChain->Present(1,0);
	else SwapChain->Present(0,0);
}

HRESULT Direct3D::CleanupRenderTarget(){
	if (renderTargetView == nullptr) return S_FALSE;
	SAFE_RELESE(renderTargetView);
	return S_OK;
}

HRESULT Direct3D::CreateRenderTarget(){
	result = SwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&BackBuffer);
	if (FAILED(result)) return result;

	result = d3d11Device->CreateRenderTargetView(BackBuffer,NULL,&renderTargetView);
	if(FAILED(result)) return result;

	return result;
}

bool Direct3D::CreateViewPort(){
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX	= 0;
	viewport.TopLeftY	= 0;
	viewport.Width		= (FLOAT)this->width;
	viewport.Height		= (FLOAT)this->height;
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

	if(FAILED(d3d11Device->CreateBuffer(&cbbd,NULL,&cbPerObjectBuffer)))
		return false;

	// Per frame buffer
	ZeroMemory(&cbbd,sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage			= D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth		= sizeof(bpd::cbPerFrame);
	cbbd.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags		= 0;

	if(FAILED(d3d11Device->CreateBuffer(&cbbd,NULL,&cbPerFrameBuffer)))
		return false;

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
	if(FAILED(d3d11Device->CreateSamplerState(&sampDesc,&LinearSamplerState)))
		return false;

	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc,sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode					= D3D11_FILL_SOLID;
	cmdesc.CullMode					= D3D11_CULL_BACK;
	cmdesc.FrontCounterClockwise	= true;
	cmdesc.MultisampleEnable		= true;
	if(FAILED(d3d11Device->CreateRasterizerState(&cmdesc,&CCWcullMode)))
		return false;

	cmdesc.FrontCounterClockwise = false;

	if(FAILED(d3d11Device->CreateRasterizerState(&cmdesc,&CWcullMode)))
		return false;

	if (wireframe)
		cmdesc.FillMode = D3D11_FILL_WIREFRAME;
	cmdesc.CullMode		= D3D11_CULL_NONE;

	if(FAILED(d3d11Device->CreateRasterizerState(&cmdesc,&RSCullNone)))
		return false;

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

	if(FAILED(d3d11Device->CreateBlendState(&blendDesc,&Transparency)))
		return false;

	return true;
}