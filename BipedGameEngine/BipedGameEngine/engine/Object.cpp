#include "Object.h"
using namespace bpd;
using namespace DirectX::SimpleMath;

Object::Object() {
	m_proj = Matrix::Identity;
	m_view = Matrix::Identity;
	m_world = Matrix::Identity;
	//m_effect = nullptr;
	//m_inputLayout = nullptr;
}
Object::~Object() {}

bool Object::Initialize(PRIMITIVE_OBJ type, ID3D11DeviceContext * d3d11DevCon, int width, int height) {
	switch (type) {
	case bpd::Object::Cube:
		m_shape = GeometricPrimitive::CreateCube(d3d11DevCon);
		break;
	case bpd::Object::Cone:
		m_shape = GeometricPrimitive::CreateCone(d3d11DevCon);
		break;
	case bpd::Object::Torus:
		m_shape = GeometricPrimitive::CreateTorus(d3d11DevCon);
		break;
	case bpd::Object::Sphere:
		m_shape = GeometricPrimitive::CreateSphere(d3d11DevCon);
		break;
	case bpd::Object::Teapot:
		m_shape = GeometricPrimitive::CreateTeapot(d3d11DevCon);
		break;
	case bpd::Object::Cylinder:
		m_shape = GeometricPrimitive::CreateCylinder(d3d11DevCon);
		break;
	case bpd::Object::Dodecahedron:
		m_shape = GeometricPrimitive::CreateDodecahedron(d3d11DevCon);
		break;
	default:
		break;
	}
	m_world = Matrix::Identity;
	m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
		Vector3::Zero, Vector3::UnitY);
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
		float(width) / float(height), 0.1f, 10.f);

	return true;
}
bool Object::Initialize() {
	return true;
}

void Object::Render() {
	//if (m_effect == nullptr)
	m_shape->Draw(m_world, m_view, m_proj);
}

void Object::Shutdown() {
	m_shape.reset();
}

void Object::SetEffect(
	DirectX::BasicEffect * effect,
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout,
	ID3D11Device * d3d11Dev
) {
	/*m_effect = effect;
	m_inputLayout = inputLayout;

	m_shape->CreateInputLayout(m_effect,
		m_inputLayout.ReleaseAndGetAddressOf());
		*/
}