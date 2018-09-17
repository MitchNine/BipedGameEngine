#include "Scene.h"

using namespace bpd;
using namespace DirectX::SimpleMath;

Scene::Scene() {
	objects = std::vector<Object*>();
	timer = 0;
}
Scene::~Scene() {}

bool Scene::Initialize(ID3D11DeviceContext * d3d11DevCon, int width, int height, ID3D11Device * d3d11Dev) {
	Object* obj = new Object();
	objects.push_back(obj);
	objects.back()->Initialize(Object::PRIMITIVE_OBJ::Sphere, d3d11DevCon, width, height);
	
	return true;
}

bool Scene::AddModel(std::string path) {
	
	return true;
}

void Scene::Render() {
	for (int i = 0; i < objects.size(); i++)
		objects[i]->Render();
}

void Scene::Update(double td) {
	timer += 0.01f;
}

void Scene::Shutdown() {
	for (int i = 0; i < objects.size(); i++)
		objects[i]->Shutdown();
}