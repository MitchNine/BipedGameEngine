#include "Shader.h"
using namespace bpd;

void PrintCompileErrorMessages(ID3DBlob* errors) {
	// Print the compile error message into a message box
	MessageBox(0,(WCHAR *)errors->GetBufferPointer(),L"Error",MB_OK | MB_ICONERROR);
}

HRESULT CompileShader(const WCHAR *filename,const char *entrypoint,const char *profile,ID3DBlob **out_code) {
	ID3DBlob *errors;

	// Compile the shader
	HRESULT hresult = D3DCompileFromFile(filename,0,0,entrypoint,profile,D3DCOMPILE_DEBUG,0,out_code,&errors);
	
	// If compile failed, print the error out into a message box
	if(hresult != S_OK) {
		PrintCompileErrorMessages(errors);
	}
	return hresult;
}

Shader::Shader() {
	// Vertex and Pixel shaders
	VS = 0;
	PS = 0;
	VS_Buffer = 0;
	PS_Buffer = 0;

	// Layout
	inputLayout = 0;
}
Shader::~Shader() {}

bool Shader::Initialize(
	std::string vs_path,
	std::string vs_entry,
	std::string ps_path,
	std::string ps_entry,
	UINT		numElements,
	D3D11_INPUT_ELEMENT_DESC	* layout,
	ID3D11Device				* d3d11Device,
	ID3D11DeviceContext			* d3d11DevCon
){
	ID3DBlob *errors;

	// Compile vertex shader from shader file
	result = D3DCompileFromFile(
		StringConverter::StringToWide(vs_path).c_str(),
		0,0,vs_entry.c_str(),"vs_4_0",D3DCOMPILE_DEBUG,0,&VS_Buffer,&errors);
	if(FAILED(result)) {
		MessageBox(0,(WCHAR *)errors->GetBufferPointer(),L"Error",MB_OK | MB_ICONERROR);
		return false;
	}

	// Compile pixel shader from shader file
	result = D3DCompileFromFile(
		StringConverter::StringToWide(ps_path).c_str(),
		0,0,ps_entry.c_str(),"ps_4_0",D3DCOMPILE_DEBUG,0,&PS_Buffer,&errors);
	if(FAILED(result)) {
		MessageBox(0,(WCHAR *)errors->GetBufferPointer(),L"Error",MB_OK | MB_ICONERROR);
		return false;
	}

	// Create the Shader Objects
	result = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(),VS_Buffer->GetBufferSize(),NULL,&VS);
	if(FAILED(result)) return false;
	result = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(),PS_Buffer->GetBufferSize(),NULL,&PS);
	if(FAILED(result)) return false;

	// Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS,0,0);
	d3d11DevCon->PSSetShader(PS,0,0);

	// Create the Input Layout
	result = d3d11Device->CreateInputLayout(
		layout,
		numElements,
		VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(),
		&inputLayout
	);
	if(FAILED(result)) return false;

	// Set the Input Layout
	d3d11DevCon->IASetInputLayout(inputLayout);

	// Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

bool Shader::Initialize(
	ID3D11Device				* d3d11Device,
	ID3D11DeviceContext			* d3d11DevCon
) {
	ID3DBlob *errors;

	// Compile vertex shader from shader file
	result = D3DCompileFromFile(
		L"Project\\Library\\Shaders\\Effects.fx",
		0, 0, "VS", "vs_4_0", D3DCOMPILE_DEBUG, 0, &VS_Buffer, &errors);
	if (FAILED(result)) {
		MessageBox(0, (WCHAR *)errors->GetBufferPointer(), L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Compile pixel shader from shader file
	result = D3DCompileFromFile(
		L"Project\\Library\\Shaders\\Effects.fx",
		0, 0, "PS", "ps_4_0", D3DCOMPILE_DEBUG, 0, &PS_Buffer, &errors);
	if (FAILED(result)) {
		MessageBox(0, (WCHAR *)errors->GetBufferPointer(), L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Create the Shader Objects
	result = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	if (FAILED(result)) return false;
	result = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	if (FAILED(result)) return false;

	// Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,  0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the Input Layout
	result = d3d11Device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(),
		&inputLayout
	);
	if (FAILED(result)) return false;

	// Set the Input Layout
	d3d11DevCon->IASetInputLayout(inputLayout);

	// Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}


void Shader::Shutdown(){
	// Vertex and Pixel shaders
	SAFE_RELESE(VS);
	SAFE_RELESE(PS);
	SAFE_RELESE(VS_Buffer);
	SAFE_RELESE(PS_Buffer);

	// Layout
	SAFE_RELESE(inputLayout);
}

void Shader::SetShader(ID3D11DeviceContext * d3d11DevCon){
	// Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS,0,0);
	d3d11DevCon->PSSetShader(PS,0,0);
}