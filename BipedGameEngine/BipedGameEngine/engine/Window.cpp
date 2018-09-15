#include "Window.h"

using namespace bpd;

Window::Window() {}
Window::~Window() {}

bool Window::Initialize(
	std::string window_class,
	std::string window_title,
	int width,
	int height,
	HINSTANCE hInstance
){
	RECT rc = { 0, 0, width, height };
	AdjustWindowRectEx(&rc,WS_OVERLAPPEDWINDOW,FALSE,NULL);
	windowWidth		= width;
	windowHeight	= height;
	screenWidth		= rc.right - rc.left;
	screenHeight	= rc.bottom - rc.top;
	this->window_title = StringConverter::StringToWide(window_title);
	this->window_class = StringConverter::StringToWide(window_class);
	
	if (!bpd_RegisterWindow(hInstance))
		return false;
	if (!bpd_CreateWindow(hInstance))
		return false;

	ShowWindow(hwnd,SW_SHOW);
	UpdateWindow(hwnd);

	return true;
}

bool Window::bpd_RegisterWindow( HINSTANCE hInstance ){
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = window_class.c_str();
	wc.hIconSm = LoadIcon(NULL,IDI_APPLICATION);

	if(!RegisterClassEx(&wc)) {
		ErrorLogger::Log("Failed to register class");
		return false;
	}

	return true;
}
bool Window::bpd_CreateWindow(HINSTANCE hInstance){
	hwnd = CreateWindowEx(
		NULL,
		window_class.c_str(),
		window_title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,
		screenWidth,
		screenHeight,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if(!hwnd) {
		ErrorLogger::Log("Failed to create window");
		return false;
	}

	return true;
}