#pragma once
#include "core/pch.h"

namespace bpd {
	class Input {
	public:
		Input();
		~Input();

		bool Initialize(HINSTANCE,HWND,int,int);
		void Shutdown();
		bool Update();

		void GetMousePosition(int&,int&);
		void GetMouseMovement(int&,int&);

		bool GetMouseKey(int);
		bool GetMouseKeyDown(int);
		bool GetKey(unsigned char key);


	private:
		bool ReadKeyboard();
		bool ReadMouse();
		void ProcessInput();
	private:
		IDirectInput8		*m_directInput;
		IDirectInputDevice8	*m_keyboard;
		IDirectInputDevice8	*m_mouse;

		unsigned char m_keyboardState[256];
		DIMOUSESTATE m_mouseState;

		int m_screenWidth,m_screenHeight;
		int m_mouseX,m_mouseY;

		int pinstate[8];
	};

}