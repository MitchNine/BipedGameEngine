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
		&device,
		StringConverter::StringToWide(dir),
		StringConverter::StringToWide(name),
		&swapChain, true, true
	);

	models.push_back(m);

	return m;
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
		models[i]->Shutdown();
}