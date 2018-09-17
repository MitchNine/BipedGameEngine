#include "Scene.h"

using namespace bpd;
using namespace DirectX::SimpleMath;

Scene::Scene() {
	models = std::vector<Model>();
	timer = 0;
}
Scene::~Scene() {}

bool Scene::Initialize(ID3D11DeviceContext * d3d11DevCon, int width, int height) {
	m_shape = GeometricPrimitive::CreateTorus(d3d11DevCon);

	m_world = Matrix::Identity;

	m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
		Vector3::Zero, Vector3::UnitY);
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
		float(width) / float(height), 0.1f, 10.f);
	return true;
}

bool Scene::AddModel(std::string path) {
	
	return true;
}

void Scene::Render() {
	m_shape->Draw(m_world, m_view, m_proj);
}

void Scene::Update(float td) {
	timer += 0.01f;
	m_world = Matrix::CreateRotationZ(2.f + timer);
}

void Scene::Shutdown() {
	models.clear();
	m_shape.reset();
}