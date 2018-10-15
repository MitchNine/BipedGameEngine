#pragma once
#include "core/pch.h"

namespace bpd{
	class Shader {
	public:
		Shader();
		~Shader();

		bool Initialize(
			std::string vs_path,
			std::string vs_entry,
			std::string ps_path,
			std::string ps_entry,
			UINT		numElements,
			D3D11_INPUT_ELEMENT_DESC	* layout,
			ID3D11Device				* d3d11Device,
			ID3D11DeviceContext			* d3d11DevCon
		);
		bool Initialize(
			ID3D11Device				* d3d11Device,
			ID3D11DeviceContext			* d3d11DevCon
		);
		void Shutdown();
		void SetShader(ID3D11DeviceContext * d3d11DevCon);

	private:
		HRESULT result;

		// Vertex and Pixel shaders
		ID3D11VertexShader			* VS;
		ID3D11PixelShader			* PS;
		ID3D10Blob					* VS_Buffer;
		ID3D10Blob					* PS_Buffer;

		// Input layout
		D3D11_INPUT_ELEMENT_DESC	* layout;
		ID3D11InputLayout			* inputLayout;
	};
}
