#pragma once
#include "core\pch.h"
#include "Direct3D.h"
#include "Camera.h"
#include <fstream>
#include <istream>

namespace bpd {
	struct TextureManager {
		std::vector<ID3D11ShaderResourceView*> TextureList;
		std::vector<std::wstring> TextureNameArray;     // So we don't load in the same texture twice
	};

	struct SurfaceMaterial {
		std::wstring MatName;   // So we can match the subset with it's material
		DirectX::XMFLOAT4 Diffuse;		// Surface's colors
		DirectX::XMFLOAT3 Ambient;		// Transparency (Alpha) stored in 4th component
		DirectX::XMFLOAT4 Specular;      // Specular power stored in 4th component

		// Texture ID's to look up texture in SRV array
		int DiffuseTextureID;
		int AmbientTextureID;
		int SpecularTextureID;
		int AlphaTextureID;
		int NormMapTextureID;

		// Booleans so we don't implement techniques we don't need
		bool HasDiffTexture;
		bool HasAmbientTexture;
		bool HasSpecularTexture;
		bool HasAlphaTexture;
		bool HasNormMap;
		bool IsTransparent;
	};
	struct ObjModel {
		int Subsets = 0;                    // Number of subsets in obj model
		DirectX::XMMATRIX World = DirectX::XMMATRIX();	// Models world matrix
		ID3D11Buffer* VertBuff;             // Models vertex buffer
		ID3D11Buffer* IndexBuff;            // Models index buffer
		std::vector<int> SubsetIndexStart;  // Subset's index offset
		std::vector<int> SubsetMaterialID;  // Lookup ID for subsets surface material
		std::vector<DWORD> Indices;         // Models index list
		std::vector<DirectX::XMFLOAT3> Vertices;		// Models vertex positions list
		std::vector<DirectX::XMFLOAT3> AABB;			// Stores models AABB (min vertex, max vertex, and center)
											// Where AABB[0] is the min Vertex, and AABB[1] is the max vertex
		DirectX::XMFLOAT3 Center;			// True center of the model
		float BoundingSphere;				// Model's bounding sphere
	};
	struct Transform {
		DirectX::XMMATRIX MRotation;				// Rotation Matrix
		DirectX::XMMATRIX MScale;					// Scale Matrix
		DirectX::XMMATRIX MTranslation;				// Translation Matrix

		DirectX::XMFLOAT3 position;					// Vector3 to translate the Translation Matrix
		DirectX::XMFLOAT3 scail;					// Vector3 to scail the Scale Matrix
		DirectX::XMFLOAT3 rotation;					// Vector3 to rotate the Rotation Matrix
	};

	class Model {
	public:
		Model();
		~Model();

		bool Initialize();
		void Render(
			Direct3D* d3d,						// Pointer to direct 3D
			Camera* cam,						// Pointer to the camera
			UINT stride,						// stride of the vertex
			UINT offset,						// offset of the vertex
			cbPerObject& cbPerObj				// Reference to the pre object struct
		);
		void Render_gizmos(
			Direct3D* d3d,						// Pointer to direct 3D
			Camera* cam,						// Pointer to the camera
			UINT stride,						// stride of the vertex
			UINT offset,						// offset of the vertex
			cbPerObject_gizmos& cbPerObj,		// Reference to the pre object struct
			DirectX::XMFLOAT4 color
		);
		void Update();
		void Shutdown();

		HRESULT CreateTexture(
			ID3D11Device* device,				// Pointer to the device
			const WCHAR* filename,				// shader filename (Effects.fx)
			ID3D11ShaderResourceView **texture	// Pointer to the shader resource
		);

		bool LoadObjModel(
			ID3D11Device* device,		// Pointer to the device
			std::wstring path,			// object path
			std::wstring Filename,      // obj m_model filename (m_model.obj)
			IDXGISwapChain* SwapChain,	// Pointer to the swap chain
			bool IsRHCoordSys,          // True if m_model was created in right hand coord system
			bool ComputeNormals			// True to compute the normals, false to use the files normals
		);

		bool LoadObjModel(
			ID3D11Device* device,		// Pointer to the device
			std::wstring path,			// object path
			std::wstring Filename,      // obj m_model filename (m_model.obj)
			IDXGISwapChain* SwapChain,	// Pointer to the swap chain
			bool IsRHCoordSys,          // True if m_model was created in right hand coord system
			bool ComputeNormals,		// True to compute the normals, false to use the files normals
			std::vector<DirectX::XMFLOAT3> AABB
		);


		Transform						transform;
		ObjModel						model;
	private:
		std::vector<SurfaceMaterial>	material;
		TextureManager					textureMgr;
	};

}