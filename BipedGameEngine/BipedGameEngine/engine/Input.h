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
		bool GetKey(int key);
		bool GetKeyDown(int key);

	private:
		bool ReadKeyboard();
		bool ReadMouse();
		void ProcessInput();
	private:
		const static int NumKeys = 256;

		IDirectInput8		*directInput;
		IDirectInputDevice8	*keyboard;
		IDirectInputDevice8	*mouse;
		DIMOUSESTATE mouseState;
		DIMOUSESTATE prevMouseState;

		bool keys[NumKeys];
		bool prevKeys[NumKeys];

		int screenWidth,screenHeight;
		int mouseX,mouseY;
	};

}