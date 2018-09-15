#include "System.h"
using namespace bpd;

System *System::instance = nullptr;

System::System() {
	frameCount	= 0;
	fps			= 0;
	time		= new bpd::Time;
	window		= new bpd::Window;
	direct3D	= new bpd::Direct3D;
}
System::~System() {}

bool System::Initialize(HINSTANCE hInstance){
	// Initialize win32 window
	if (!window->Initialize(
		"BPD_ENGINE_WINDOW",
		"Biped Game Engine",
		800,
		600,
		hInstance
	)){
		ErrorLogger::Log("Failed to initialize window");
		return false;
	}

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
	return msg.wParam;
}

void System::Shutdown(){
	SAFE_SHUTDOWN(direct3D);
	SAFE_DELETE(window);
}

void System::Update(double deltaTime){
	direct3D->Update(deltaTime);
}

void System::Render(){
	direct3D->Render();
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