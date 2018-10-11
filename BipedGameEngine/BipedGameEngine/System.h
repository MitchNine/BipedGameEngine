#pragma once
#include "Engine\Window.h"
#include "Engine\Direct3D.h"
#include "Engine\Shader.h"
#include "Engine\Scene.h"
#include "Engine\Input.h"

namespace bpd{
	class System {
	public:
		System();
		~System();

		bool Initialize(HINSTANCE hInstance);
		void Shutdown();
		void Update(double deltaTime);
		void Render();
		int MessageLoop();

	public:
		static System* instance;
		inline Direct3D * GetDirect3D() { return direct3D; }

	private:
		Window		* window;
		Direct3D	* direct3D;
		Shader		* shader;
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