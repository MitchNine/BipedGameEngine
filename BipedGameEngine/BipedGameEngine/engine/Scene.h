#pragma once
#include "core\pch.h"
#include "Shader.h"
#include "Model.h"

namespace bpd {
	// List of all the primitive shapes
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


	//////////////////////////////////////////////////////////////////////////
	//								SCENE
	//
	// This class has a different lists for each type of rasterizer that needs
	// to be used. In this we have three, models, gizmos, gizmos_boundingBox.
	//
	// models - Stores the standard models that are being loaded into the scene
	//			with color
	//
	// gizmos - Stores the objects that represent debug items only seen within
	//			the editor. Shouldn't show up when compiling the game
	//
	// gizmos_boundingBox - a test for showing bounding boxes for all the items
	//						in the models list
	//
	//////////////////////////////////////////////////////////////////////////

	class Scene {
	public:
		Scene();
		~Scene();

		// Initialize the scene
		// Setting up shaders to be used
		bool Initialize(Direct3D * d3d);

		// Render everything in the scene
		void Render(
			Direct3D* d3d,
			Camera* cam
		);
		// Update everything in the scene
		void Update(double dt);

		// Clear all pointers
		void Shutdown();

		// Add a new model to the scene
		Model* AddModel(
			std::string path,
			ID3D11Device* device,
			IDXGISwapChain* swapChain
		);

		// Add a new primitive shape to the scene
		Model* AddModel(
			Primitives shape,
			ID3D11Device* device,
			IDXGISwapChain* swapChain
		);

		// Return a model given the models ID within the scene
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