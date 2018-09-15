#pragma once
#include "Window.h"
#include "Direct3D.h"

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

		bpd::Time* time;
		int frameCount,fps;
		double frameTime;
	};
}