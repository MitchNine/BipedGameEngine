#include "System.h"
using namespace bpd;

System *System::instance = nullptr;

System::System() {
	frameCount	= 0;
	fps			= 0;
	frameTime	= 0;
	time		= new bpd::Time;
	window		= new bpd::Window;
	direct3D	= new bpd::Direct3D;
	shader		= new bpd::Shader;
	scene		= new bpd::Scene;
}
System::~System() {}

bool System::Initialize(HINSTANCE hInstance){
	// Initialize win32 window
	if (!window->Initialize(
		"BPD_ENGINE_WINDOW",
		"Biped Game Engine",
		800,
		800,
		hInstance
	)){
		ErrorLogger::Log("Failed to initialize window");
		return false;
	}

	// DirectX 11 setup
	direct3D->fullscreen	= false;
	direct3D->wireframe		= true;
	direct3D->vsync			= true;

	// Initialize DirectX 11
	if(!direct3D->Initialize(
		hInstance,
		window->GetWindowWidth(),
		window->GetWindowHeight(),
		window->GetHWND()
	)){
		ErrorLogger::Log("Failed to initialize DirectX");
		return false;
	}

	// Shader layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Initialize shaders
	if(!shader->Initialize(
		"assets\\shaders\\Effects.fx",
		"VS",
		"assets\\shaders\\Effects.fx",
		"PS",
		ARRAYSIZE(layout),
		layout,
		direct3D->GetDevice(),
		direct3D->GetDeviceContext()
	)) {
		ErrorLogger::Log("Failed to initialize the shader");
		return false;
	}

	// Initialize the rest of directX
	if (!direct3D->InitializeBuffers()){
		ErrorLogger::Log("Failed to initialize DirectX buffers");
		return false;
	}

	if (!scene->Initialize(direct3D->GetDeviceContext(), window->GetScreenWidth(), window->GetScreenWidth(), direct3D->GetDevice())) {
		ErrorLogger::Log("Failed to initialize scene");
		return false;
	}

	instance = this;

	MessageLoop();
	Shutdown();

	return true;
}

int System::MessageLoop() {
	MSG msg;
	ZeroMemory(&msg,sizeof(MSG));
	while(true) {
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
		);

		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if(msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			frameCount++;
			if(time->GetTime() > 1.0f) {
				fps = frameCount;
				frameCount = 0;
				time->StartTimer();
			}
			frameTime = time->GetFrameTime();
			Update(frameTime);
			Render();
		}
	}
	return (int)msg.wParam;
}

void System::Shutdown(){
	SAFE_SHUTDOWN(direct3D);
	SAFE_SHUTDOWN(shader);
	SAFE_DELETE(window);
}

void System::Update(double deltaTime){
	direct3D->Update(deltaTime);
	scene->Update(deltaTime);
}

void System::Render(){
	float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	direct3D->ClearScreen(color);
	shader->SetShader(direct3D->GetDeviceContext());
	
	scene->Render();

	direct3D->Present();
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
	case WM_SIZE:
		if(System::instance != nullptr && wParam != SIZE_MINIMIZED) {
			System::instance->GetDirect3D()->CleanupRenderTarget();
			System::instance->GetDirect3D()->GetSwapChain()->ResizeBuffers(
				0,
				(UINT)LOWORD(lParam),
				(UINT)HIWORD(lParam),
				DXGI_FORMAT_UNKNOWN,
				0
			);
			System::instance->GetDirect3D()->CreateRenderTarget();
		}
		return 0;
	case  WM_KEYDOWN:
		if(wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		if((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(
		hwnd,
		msg,
		wParam,
		lParam
	);
}