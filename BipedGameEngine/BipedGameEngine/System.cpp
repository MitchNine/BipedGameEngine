#include "System.h"
using namespace bpd;

// Set the instance to nullptr for initialization
System *System::instance = nullptr;

System::System() {
	// Initialize all the member variables
	time		= new bpd::Time;
	window		= new bpd::Window;
	direct3D	= new bpd::Direct3D;
	scene		= new bpd::Scene;
	input		= new bpd::Input;
	cam			= new bpd::Camera;
	frameCount	= 0;
	frameTime	= 0;
	fps			= 0;
	curr_mouseX = 0;
	curr_mouseY = 0;
	prev_mouseX = 0;
	prev_mouseY = 0;
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

	// DirectX 11 setup flags
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

	// Initialize keyboard and mouse input
	if(!input->Initialize(hInstance,window->GetHWND(),window->GetWindowWidth(),window->GetWindowHeight())) {
		ErrorLogger::Log("Failed to initialize input");
		return false;
	}

	// Initialize the scene
	if (!scene->Initialize(direct3D)) {
		ErrorLogger::Log("Failed to initialize scene");
		return false;
	}

	// Set up camera information
	cam->camPosition = DirectX::XMVectorSet(0.0f,2.0f,-8.0f,0.0f);
	cam->camUp = DirectX::XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	cam->camView = DirectX::XMMatrixLookAtLH(cam->camPosition,cam->camTarget,cam->camUp);
	cam->camProjection = DirectX::XMMatrixPerspectiveFovLH(0.4f*3.14159f,window->GetWindowWidth() / window->GetWindowHeight(),1.0f,1000.0f);

	// Load a primitive model into the scene
	// This model is acting as the position of the directional light
	Model* light = scene->AddModel(Primitives::CONE,direct3D->GetDevice(),direct3D->GetSwapChain());
	light->transform.position = direct3D->GETLight().pos;
	light->transform.rotation = direct3D->GETLight().dir;

	// Load a primitive model into the scene
	// This model is acting as a reference to the center of the world
	Model* center = scene->AddModel(bpd::Primitives::SPHERE,direct3D->GetDevice(),direct3D->GetSwapChain());
	center->transform.scail = DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f);

	// Load the first model of a rock into the scene
	Model* model = scene->AddModel("Project\\Assets\\Rock_9\\Rock_9.obj", direct3D->GetDevice(), direct3D->GetSwapChain());
	model->transform.position = DirectX::XMFLOAT3(0,-6,0);
	model->transform.scail = DirectX::XMFLOAT3(2, 0.5f, 2);
	
	// Load the second model of a rock into the scene
	Model* model2 = scene->AddModel("Project\\Assets\\Rock_9\\Rock_9.obj",direct3D->GetDevice(),direct3D->GetSwapChain());
	model2->transform.scail = DirectX::XMFLOAT3(2,1,2);
	model2->transform.position = DirectX::XMFLOAT3(10,0,0);

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
			// Count the time it takes for one update loop
			frameCount++;
			if(time->GetTime() > 1.0f) {
				fps = frameCount;
				frameCount = 0;
				time->StartTimer();
			}
			frameTime = time->GetFrameTime();

			// Update everything
			Update(frameTime);
			// Render everything
			Render();
		}
	}
	return (int)msg.wParam;
}

void System::Shutdown(){
	// Deallocate any pointers
	SAFE_SHUTDOWN	(input);
	SAFE_SHUTDOWN	(scene);

	SAFE_DELETE		(window);
	SAFE_DELETE		(time);
	SAFE_DELETE		(cam);

	SAFE_SHUTDOWN	(direct3D);
}

void System::Update(double deltaTime){
	// Get any new inputs
	input->Update();

	// Update anything that needs to be updated in the direct 3D class
	direct3D->Update(deltaTime);

	// Update the transforms of all items within the scene
	scene->Update(deltaTime);
	
	// Quit the application is ESC key was hit
	if(input->GetKeyDown(DIK_ESCAPE))
		PostMessage(window->GetHWND(),WM_DESTROY,0,0);

	// Allow for free camera movement when RMB is held down
	if(input->GetMouseKey(1)) {
		// Set the speed of the free camera movement
		float speed = 10.0f * deltaTime;

		// Move the camera when corresponding keys are pressed
		if(input->GetKey(DIK_LSHIFT)) speed *= 3;
		if(input->GetKey(DIK_A)) cam->moveLeftRight -= speed;
		if(input->GetKey(DIK_D)) cam->moveLeftRight += speed;
		if(input->GetKey(DIK_W)) cam->moveBackForward += speed;
		if(input->GetKey(DIK_S)) cam->moveBackForward -= speed;
		if(input->GetKey(DIK_E)) cam->moveDownUp += speed;
		if(input->GetKey(DIK_Q)) cam->moveDownUp -= speed;
		if(input->GetKey(DIK_SPACE)) cam->moveDownUp += speed;

		// Get the mouse movement for camera rotation
		input->GetMouseMovement(curr_mouseX,curr_mouseY);

		// If mouse position isn't the same as last frame, set the new rotation of the camera
		if((curr_mouseX != prev_mouseX) || (curr_mouseY != prev_mouseY)) {

			cam->camYaw += curr_mouseX * 0.001f;
			cam->camPitch += curr_mouseY * 0.001f;

			prev_mouseX = curr_mouseX;
			prev_mouseY = curr_mouseY;
		}
	}

	// Update the cameras transform
	cam->Update();
}

void System::Render(){
	// Set the clear color (dark gray)
	float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	// Clear the render target and stencil
	direct3D->ClearScreen(color);

	// Render all the models in the scene
	scene->Render(
		direct3D,
		cam
	);

	// pass everything to the GPU
	direct3D->Present();
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
	case WM_SIZE:
		if(System::instance != nullptr && wParam != SIZE_MINIMIZED) {
			// When screen is re-sized, recreate the render target with the new window size
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
		// If ESC was pressed, quit the application
		if(wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		// Disable the ALT key menu
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