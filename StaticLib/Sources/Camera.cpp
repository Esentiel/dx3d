#include "stdafx.h"
#include "Camera.h"

#include <algorithm>

#include <d3d11.h>

using namespace Library;

Camera::Camera(float fov, int width, int height, float nearPlane, float farPlane) :
	m_fov(fov),
	m_width(width),
	m_height(height),
	m_near(nearPlane),
	m_far(farPlane),
	m_viewDirty(false),
	m_viewport(new D3D11_VIEWPORT),
	m_position(new DirectX::XMFLOAT3(.0f, 30.f, 50.0f)),
	m_look(new DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f)),
	m_up(new DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)),
	m_right(new DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f))
{
	UpdateViewMatrix();
	UpdateProjection();
	UpdateViewport();
}

Camera::~Camera()
{
}

const DirectX::XMMATRIX* Camera::GetView()
{
	m_viewM.reset(new DirectX::XMMATRIX(DirectX::XMLoadFloat4x4(&m_view)));
	return m_viewM.get();
}

const DirectX::XMMATRIX* Camera::GetProjection() const
{
	return m_projection.get();
}

void Camera::UpdateProjection()
{
	float aspectRatio = (float)m_width / m_height;
	m_projection.reset(new DirectX::XMMATRIX(DirectX::XMMatrixPerspectiveFovRH(m_fov, aspectRatio, m_near, m_far)));
}

const D3D11_VIEWPORT* Library::Camera::GetViewport() const
{
	return m_viewport.get();
}

void Library::Camera::UpdateViewport()
{
	assert(g_D3D->deviceCtx);

	m_viewport->Width = (float)m_width;
	m_viewport->Height = (float)m_height;
	m_viewport->TopLeftX = 0.f;
	m_viewport->TopLeftY = 0.f;
	m_viewport->MinDepth = 0.f;
	m_viewport->MaxDepth = 1.f;

	g_D3D->deviceCtx->RSSetViewports(1, m_viewport.get());
}

DirectX::XMFLOAT3* Library::Camera::GetPosition() const
{
	return m_position.get();
}

void Library::Camera::UpdatePosition(float x, float y, float z)
{
	DirectX::XMVECTOR dirN = DirectX::XMVector4Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(m_look.get()), DirectX::XMLoadFloat3(m_position.get())));

	DirectX::XMFLOAT4 normalDir4;
	DirectX::XMStoreFloat4(&normalDir4, dirN);

	m_position->x += (normalDir4.x * x);
	m_position->y += (normalDir4.y * y);
	m_position->z += (normalDir4.z * z);
	
	m_viewDirty = true;
}

void Library::Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	DirectX::XMMATRIX R = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(m_right.get()), angle);

	DirectX::XMStoreFloat3(m_up.get(), DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(m_up.get()), R));
	DirectX::XMStoreFloat3(m_look.get(), DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(m_look.get()), R));

	m_viewDirty = true;
}

void Library::Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(angle);

	DirectX::XMStoreFloat3(m_right.get(), DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(m_right.get()), R));
	DirectX::XMStoreFloat3(m_look.get(), DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(m_look.get()), R));
	DirectX::XMStoreFloat3(m_look.get(), DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(m_look.get()), R));

	m_viewDirty = true;
}

void Library::Camera::UpdateViewMatrix()
{
	if (m_viewDirty)
	{
		DirectX::XMVECTOR R = DirectX::XMLoadFloat3(m_right.get());
		DirectX::XMVECTOR U = DirectX::XMLoadFloat3(m_up.get());
		DirectX::XMVECTOR L = DirectX::XMLoadFloat3(m_look.get());
		DirectX::XMVECTOR P = DirectX::XMLoadFloat3(m_position.get());

		// Keep camera's axes orthogonal to each other and of unit length.
		L = DirectX::XMVector3Normalize(L);
		U = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = DirectX::XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, R));
		float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, U));
		float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, L));

		DirectX::XMStoreFloat3(m_right.get(), R);
		DirectX::XMStoreFloat3(m_up.get(), U);
		DirectX::XMStoreFloat3(m_look.get(), L);

		m_view(0, 0) = m_right->x;
		m_view(1, 0) = m_right->y;
		m_view(2, 0) = m_right->z;
		m_view(3, 0) = x;

		m_view(0, 1) = m_up->x;
		m_view(1, 1) = m_up->y;
		m_view(2, 1) = m_up->z;
		m_view(3, 1) = y;

		m_view(0, 2) = m_look->x;
		m_view(1, 2) = m_look->y;
		m_view(2, 2) = m_look->z;
		m_view(3, 2) = z;

		m_view(0, 3) = 0.0f;
		m_view(1, 3) = 0.0f;
		m_view(2, 3) = 0.0f;
		m_view(3, 3) = 1.0f;

		m_viewDirty = false;
	}
}