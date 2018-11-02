#pragma once
#include "core/ErrorLogger.h"
#include "Time.h"

// WndProc callback 
LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);


namespace bpd{
	class System;
	
	class Window {
	public:
		Window();
		~Window();

		// Initialize the Win32 Window
		bool Initialize(
			std::string window_class,
			std::string window_title,
			int width,
			int height,
			HINSTANCE hInstance
		);

		// Return the window height
		inline int GetWindowHeight()	{ return windowHeight; }
		// Return the window height
		inline int GetWindowWidth()		{ return windowWidth;  }

		// Return the drawable application screen height
		inline int GetScreenHeight()	{ return screenHeight; }
		// Return the drawable application screen width
		inline int GetScreenWidth()		{ return screenWidth;  }

		// Return the windows handle
		inline HWND GetHWND()			{ return hwnd; }

	private:
		// Register the Win32 window
		bool bpd_RegisterWindow( HINSTANCE hInstance );
		// Create the Win32 window
		bool bpd_CreateWindow( HINSTANCE hInstance );

	private:
		HWND hwnd;
		std::wstring window_title;
		std::wstring window_class;
		int windowWidth;
		int windowHeight;
		int screenWidth;
		int screenHeight;
	};
}