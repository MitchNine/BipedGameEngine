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
	cam			= new bpd::Camera;
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
	direct3D->wireframe		= false;
	direct3D->vsync			= false;

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

	// Set up the layout for the shader
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Initialize the shader
	if (!shader->Initialize(
		"Assets\\shaders\\Effects.fx",
		"VS",
		"Assets\\shaders\\Effects.fx",
		"PS",
		ARRAYSIZE(layout),
		layout,
		direct3D->GetDevice(),
		direct3D->GetDeviceContext()
	)){
		ErrorLogger::Log("Failed to shader");
		return false;
	}

	// Initialize the scene
	if (!scene->Initialize()) {
		ErrorLogger::Log("Failed to initialize scene");
		return false;
	}

	// Camera information
	cam->camPosition = XMVectorSet(0.0f,2.0f,-8.0f,0.0f);
	cam->camTarget = XMVectorSet(0.0f,0.5f,0.0f,0.0f);
	cam->camUp = XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	cam->camView = XMMatrixLookAtLH(cam->camPosition,cam->camTarget,cam->camUp);
	cam->camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14159f,(float)window->GetWindowWidth() / (float)window->GetWindowHeight(),1.0f,1000.0f);

	// Load up the models
	Model* rock = scene->AddModel("Assets\\ground\\ground.obj", direct3D->GetDevice(), direct3D->GetSwapChain());
	rock->transform.position = XMFLOAT3(0, 0, 0);

	// Set the system instance for static access
	instance = this;

	// Start the message loop
	MessageLoop();

	// Shutdown and release all the objects
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
	SAFE_SHUTDOWN	(direct3D);
	SAFE_SHUTDOWN	(shader);
	SAFE_DELETE		(window);
}

void System::Update(double deltaTime){
	direct3D->Update(deltaTime);
	scene->Update(deltaTime);
	cam->Update();
}

void System::Render(){
	float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	// Clear the render target and stencil
	direct3D->ClearScreen(color);

	// Set the shader for the models to use
	shader->SetShader(direct3D->GetDeviceContext());

	scene->GetModel(0)->Render(direct3D->GetDeviceContext(), cam, 56, 0, direct3D->GetCBPerObjectBuffer(), cbPerObj);

	// Render all the models in the scene
	scene->Render(
		direct3D->GetDeviceContext(),
		cam,
		direct3D->GetCBPerFrameBuffer()
	);

	// pass everything to the GPU
	direct3D->Present();
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
	case WM_SIZE:
		if(System::instance != nullptr && wParam != SIZE_MINIMIZED) {

			// Recreate the render target with the new window size
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