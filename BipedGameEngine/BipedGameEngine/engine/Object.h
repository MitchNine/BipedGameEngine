#pragma once
#include "core\pch.h"
#include <Model.h>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include <Effects.h>

namespace bpd {
	class Object {
	public:
		enum PRIMITIVE_OBJ {
			Cube,
			Cone,
			Torus,
			Sphere,
			Teapot,
			Cylinder,
			Dodecahedron,
		};
	public:
		Object();
		~Object();

		bool Initialize(PRIMITIVE_OBJ type, ID3D11DeviceContext * d3d11DevCon, int width, int height);
		bool Initialize();
		void SetEffect(
			DirectX::BasicEffect * effect,
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout,
			ID3D11Device * d3d11Dev
		);
		void Render();
		void Shutdown();
	private:

		std::vector<Model> models;

		DirectX::SimpleMath::Matrix m_world;
		DirectX::SimpleMath::Matrix m_view;
		DirectX::SimpleMath::Matrix m_proj;
		std::unique_ptr<DirectX::GeometricPrimitive> m_shape;

		//DirectX::BasicEffect * m_effect;
		//Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	};

}