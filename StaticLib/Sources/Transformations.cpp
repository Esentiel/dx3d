#include "stdafx.h"
#include "Transformations.h"

#include <DirectXMath.h>

using namespace Library;

Transformations::Transformations() :
	m_model(std::make_unique<DirectX::XMMATRIX>()),
	m_position(std::make_unique<DirectX::XMFLOAT3>()),
	m_rotation(std::make_unique<DirectX::XMFLOAT3>()),
	m_scale(std::make_unique<DirectX::XMFLOAT3>(1.0f, 1.0f, 1.0f))
{
}


Transformations::~Transformations()
{
}

DirectX::XMMATRIX* Transformations::GetModel()
{
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(m_position.get()));
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_scale->x, m_scale->y, m_scale->z);
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(m_rotation.get()));

	*m_model = translation * rotation * scale;

	return m_model.get();
}

void Transformations::Move(const DirectX::XMFLOAT3 &direction)
{
	m_position->x = direction.x;
	m_position->y = direction.y;
	m_position->z = direction.z;
}

void Transformations::Rotate(const DirectX::XMFLOAT3 &rotation)
{
	m_rotation->x = rotation.x;
	m_rotation->y = rotation.y;
	m_rotation->z = rotation.z;
}

void Transformations::Scale(const DirectX::XMFLOAT3 &scale)
{
	m_scale->x = scale.x;
	m_scale->y = scale.y;
	m_scale->z = scale.z;
}
