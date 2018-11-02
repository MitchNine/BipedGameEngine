#pragma once
#include "Engine\Window.h"
#include "Engine\Direct3D.h"
#include "Engine\Scene.h"
#include "Engine\Input.h"

namespace bpd{
	class System {
	public:
		System();
		~System();

		// Initialize the all the systems within this application
		bool Initialize(HINSTANCE hInstance);
		// Clear all pointers within this application
		void Shutdown();
		// Update all objects within this application
		void Update(double deltaTime);
		// Render all objects within this application
		void Render();
		// This is the update loop for this application
		int MessageLoop();

	public:
		// Static instance of this system for access outside
		static System* instance;
		// Returns the Direct3D class, this handles all the DirectX 11 setup
		inline Direct3D * GetDirect3D() { return direct3D; }

	private:
		Window		* window;
		Direct3D	* direct3D;
		Scene		* scene;
		Input		* input;
		Time		* time;
		Camera		* cam;
		cbPerObject cbPerObj;

		int curr_mouseX = 0,curr_mouseY = 0;
		int prev_mouseX = 0,prev_mouseY = 0;

		int frameCount,fps;
		double frameTime;
	};
}