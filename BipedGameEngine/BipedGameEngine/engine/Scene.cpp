#include "Scene.h"
using namespace bpd;

Scene::Scene() {
	models = std::vector<Model*>();
	gizmos = std::vector<Model*>();
	gizmos_boundingBox = std::vector<Model*>();
	shader = new bpd::Shader;
	shader_gizmos = new bpd::Shader;
	timer = 0;
}
Scene::~Scene() {}

bool Scene::Initialize(Direct3D * d3d) {

	// Set up the layout for the shader
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,  0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Initialize the shader
	if (!shader->Initialize(
		"Project\\Assets\\shaders\\Effects.fx",
		"VS",
		"Project\\Assets\\shaders\\Effects.fx",
		"PS",
		ARRAYSIZE(layout),
		layout,
		d3d->GetDevice(),
		d3d->GetDeviceContext()
	)) {
		ErrorLogger::Log("Failed to shader");
		return false;
	}

	// Set up the layout for the shader
	D3D11_INPUT_ELEMENT_DESC layout_gizmos[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Initialize the shader
	if (!shader_gizmos->Initialize(
		"Project\\Assets\\shaders\\SolidColor.fx",
		"VS",
		"Project\\Assets\\shaders\\SolidColor.fx",
		"PS",
		ARRAYSIZE(layout_gizmos),
		layout_gizmos,
		d3d->GetDevice(),
		d3d->GetDeviceContext()
	)) {
		ErrorLogger::Log("Failed to shader");
		return false;
	}

	return true;
}

Model* Scene::AddModel(std::string path, ID3D11Device * device, IDXGISwapChain * swapChain) {
	Model *m = new Model;

	std::string dir, name;
	int i = path.size() - 1;
	for (; i > 0; i--) {
		if (path[i] == '\\')
			break;
	}

	dir = path.substr(0, i + 1);
	name = path.substr(i + 1, path.size() - 1);
	m->LoadObjModel(
		device,
		StringConverter::StringToWide(dir),
		StringConverter::StringToWide(name),
		swapChain, true, true
	);

	models.push_back(m);

	Model *m_boundingBox = new Model;
	m_boundingBox->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"cube.obj", swapChain, true, true);
	m_boundingBox->transform.position = m->transform.position;
	m_boundingBox->transform.scail = m->model.AABB[1];
	gizmos_boundingBox.push_back(m_boundingBox);

	return m;
}

Model* Scene::AddModel(
	Primitives shape,
	ID3D11Device* device,
	IDXGISwapChain* swapChain
) {
	Model *model = new Model;

	switch (shape) {
	case bpd::CONE:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"cone.obj", swapChain, true, true);
		break;
	case bpd::CUBE:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"cube.obj", swapChain, true, true);
		break;
	case bpd::CYLINDER:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"cylinder.obj", swapChain, true, true);
		break;
	case bpd::DISC:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"disc.obj", swapChain, true, true);
		break;
	case bpd::PLANE:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"plane.obj", swapChain, true, true);
		break;
	case bpd::PLATONIC:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"platonic.obj", swapChain, true, true);
		break;
	case bpd::SPHERE:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"sphere.obj", swapChain, true, true);
		break;
	case bpd::TORUS:
		model->LoadObjModel(device, L"Project\\Library\\Primitive\\", L"torus.obj", swapChain, true, true);
		break;
	default:
		break;
	}

	gizmos.push_back(model);

	return model;
}

Model* Scene::GetModel(int id){
	return models[id];
}

void Scene::Render(
	Direct3D* d3d,
	Camera* cam
) {
	// Set the shader for the models to use
	shader->SetShader(d3d->GetDeviceContext());

	d3d->SetRasterizerState(Direct3D::eRSCullNone);
	for (int i = 0; i < models.size(); i++) {
		models[i]->Render(
			d3d,				// Pointer to direct 3D
			cam,				// Pointer to the camera
			sizeof(bpd::Vertex),// Vertex stride
			0,					// Vertex offset
			cbPerObj			// Reference to the pre object struct
		);
	}

	shader_gizmos->SetShader(d3d->GetDeviceContext());

	d3d->SetRasterizerState(Direct3D::eRSCullNone);
	for (int i = 0; i < gizmos.size(); i++) {
		gizmos[i]->Render_gizmos(
			d3d,				// Pointer to direct 3D
			cam,				// Pointer to the camera
			sizeof(bpd::Vertex),// Vertex stride
			0,					// Vertex offset
			cbPerObj_gizmos,	// Reference to the pre object struct
			DirectX::XMFLOAT4(1, 1, 1, 1) // Color
		);
	}

	d3d->SetRasterizerState(Direct3D::eRSCullNoneWF);
	for (int i = 0; i < gizmos_boundingBox.size(); i++) {
		gizmos_boundingBox[i]->Render_gizmos(
			d3d,				// Pointer to direct 3D
			cam,				// Pointer to the camera
			sizeof(bpd::Vertex),// Vertex stride
			0,					// Vertex offset
			cbPerObj_gizmos,	// Reference to the pre object struct
			DirectX::XMFLOAT4(0, 1, 0, 1) // Color
		);
	}
}

void Scene::Update(double dt) {
	timer += 0.01f;
	for(int i = 0; i < models.size(); i++)
		models[i]->Update();

	for (int i = 0; i < gizmos.size(); i++)
		gizmos[i]->Update();

	for (int i = 0; i < gizmos_boundingBox.size(); i++)
		gizmos_boundingBox[i]->Update();

}

void Scene::Shutdown() {
	for(int i = 0; i < models.size(); i++)
		SAFE_SHUTDOWN(models[i]);

	for (int i = 0; i < gizmos.size(); i++)
		SAFE_SHUTDOWN(gizmos[i]);

	for (int i = 0; i < gizmos_boundingBox.size(); i++)
		SAFE_SHUTDOWN(gizmos_boundingBox[i]);


	SAFE_SHUTDOWN(shader);
	SAFE_SHUTDOWN(shader_gizmos);
}