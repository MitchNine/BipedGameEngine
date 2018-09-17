#pragma once
#include "core\pch.h"
#include <Model.h>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

namespace bpd {
	class Scene {
	public:
		Scene();
		~Scene();

		bool Initialize(ID3D11DeviceContext * d3d11DevCon, int width, int height);
		bool AddModel(std::string path);
		void Render();
		void Update(float td);
		void Shutdown();

	private:
		std::vector<Model> models;

		DirectX::SimpleMath::Matrix m_world;
		DirectX::SimpleMath::Matrix m_view;
		DirectX::SimpleMath::Matrix m_proj;
		std::unique_ptr<DirectX::GeometricPrimitive> m_shape;

		float timer;
	};
}