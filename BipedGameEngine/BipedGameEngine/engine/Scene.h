#pragma once
#include "core\pch.h"
#include "Shader.h"
#include "Model.h"

namespace bpd {
	enum Primitives {
		CONE,
		CUBE,
		CYLINDER,
		DISC,
		PLANE,
		PLATONIC,
		SPHERE,
		TORUS,
	};

	class Scene {
	public:
		Scene();
		~Scene();

		bool Initialize(Direct3D * d3d);
		void Render(
			Direct3D* d3d,
			Camera* cam
		);
		void Update(double dt);
		void Shutdown();

		Model* AddModel(
			std::string path,
			ID3D11Device* device,
			IDXGISwapChain* swapChain
		);
		Model* AddModel(
			Primitives shape,
			ID3D11Device* device,
			IDXGISwapChain* swapChain
		);
		Model* GetModel(int id);

	private:
		cbPerObject				cbPerObj;
		cbPerObject_gizmos		cbPerObj_gizmos;

		std::vector<Model*>		models;
		std::vector<Model*>		gizmos;
		std::vector<Model*>		gizmos_boundingBox;

		Shader		* shader;
		Shader		* shader_gizmos;
		float timer;
	};
}