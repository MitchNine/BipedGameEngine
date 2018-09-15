#pragma once
#include "core/ErrorLogger.h"
#include "Time.h"

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

		bool Initialize(
			std::string window_class,
			std::string window_title,
			int width,
			int height,
			HINSTANCE hInstance
		);

		inline int GetWindowHeight()	{ return windowHeight; }
		inline int GetWindowWidth()		{ return windowWidth;  }
		inline int GetScreenHeight()	{ return screenHeight; }
		inline int GetScreenWidth()		{ return screenWidth;  }
		inline HWND GetHWND()			{ return hwnd; }

	private:
		bool bpd_RegisterWindow( HINSTANCE hInstance );
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