#include "Scene.h"
using namespace bpd;

Scene::Scene() {
	models = std::vector<Model*>();
	timer = 0;
}
Scene::~Scene() {}

bool Scene::Initialize() {

	return true;
}

Model* Scene::AddModel(std::string path, ID3D11Device * device, IDXGISwapChain * swapChain) {
	Model *m = new Model;
	
	std::string dir, name;
	int i = path.size() - 1;
	for (; i > 0; i--){
		if (path[i] == '\\')
			break;
	}

	dir = path.substr(0, i + 1);
	name = path.substr(i + 1,path.size() - 1);
	m->LoadObjModel(
		device,
		StringConverter::StringToWide(dir),
		StringConverter::StringToWide(name),
		swapChain, true, true
	);

	models.push_back(m);

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

	models.push_back(model);

	return model;
}

Model* Scene::GetModel(int id){
	return models[id];
}

void Scene::Render(
	ID3D11DeviceContext* DevCon,
	Camera* cam,
	ID3D11Buffer* cbPerObjectBuffer
) {
	for (int i = 0; i < models.size(); i++)
		models[i]->Render(
			DevCon,				// Pointer to the device context
			cam,				// Pointer to the camera
			sizeof(bpd::Vertex),// Vertex stride
			0,					// Vertex offset
			cbPerObjectBuffer,	// Pointer to the per object buffer
			cbPerObj			// Reference to the pre object struct
		);
}

void Scene::Update(double dt) {
	timer += 0.01f;
	for(int i = 0; i < models.size(); i++)
		models[i]->Update();

}

void Scene::Shutdown() {
	for(int i = 0; i < models.size(); i++)
		SAFE_SHUTDOWN(models[i]);
}