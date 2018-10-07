#pragma once
#include "core\pch.h"
#include "Model.h"

namespace bpd {
	class Scene {
	public:
		Scene();
		~Scene();

		bool Initialize();
		void Render(
			ID3D11DeviceContext* DevCon,
			Camera* cam,
			ID3D11Buffer* cbPerObjectBuffer
		);
		void Update(double dt);
		void Shutdown();

		Model* AddModel(
			std::string path,
			ID3D11Device* device,
			IDXGISwapChain* swapChain
		);
		Model* GetModel(int id);

	private:
		cbPerObject cbPerObj;

		std::vector<Model*> models;
		float timer;
	};
}