#pragma once
#include "core\pch.h"
#include "Object.h"
#include <Model.h>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

namespace bpd {
	class Scene {
	public:
		Scene();
		~Scene();

		bool Initialize(ID3D11DeviceContext * d3d11DevCon, int width, int height, ID3D11Device * d3d11Dev);
		bool AddModel(std::string path);
		void Render();
		void Update(double td);
		void Shutdown();

	private:
		std::vector<Object*> objects;
		float timer;
	};
}