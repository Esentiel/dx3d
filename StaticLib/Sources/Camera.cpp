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
	m_viewDirty(true),
	m_viewport(new D3D11_VIEWPORT),
	m_position(new DirectX::XMVECTOR(DirectX::XMVectorSet(.0f, 30.f, 40.0f, 1.f))),
	m_look(new DirectX::XMVECTOR(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.f))),
	m_up(new DirectX::XMVECTOR(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.f))),
	m_right(new DirectX::XMVECTOR(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.f))),
	m_view(new DirectX::XMMATRIX)
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
	return m_view.get();
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

DirectX::XMVECTOR* Library::Camera::GetPosition() const
{
	return m_position.get();
}

void Library::Camera::Move(MoveDir dir)
{
	DirectX::XMVECTOR dirN;

	switch (dir)
	{
	case MoveDir::Forward:
		dirN = *m_look;
		break;
	case MoveDir::Backward:
		dirN = DirectX::XMVectorNegate(*m_look);
		break;
	case MoveDir::Right:
		dirN = DirectX::XMVectorNegate(*m_right);
		break;
	case MoveDir::Left:
		dirN = *m_right;
		break;
	case MoveDir::Up:
		dirN = DirectX::XMVectorNegate(*m_up);
		break;
	case MoveDir::Down:
		dirN = *m_up;
		break;
	}

	dirN = DirectX::XMVector3Normalize(dirN);

	*m_position = DirectX::XMVectorAdd(*m_position, dirN);

	m_viewDirty = true;
}

void Library::Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	DirectX::XMMATRIX R = DirectX::XMMatrixRotationAxis(*m_right, angle);

	*m_up = DirectX::XMVector3TransformNormal(*m_up, R);
	*m_look = DirectX::XMVector3TransformNormal(*m_look, R);

	m_viewDirty = true;
}

void Library::Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(angle);

	*m_right = DirectX::XMVector3TransformNormal(*m_right, R);
	*m_look =  DirectX::XMVector3TransformNormal(*m_look, R);

	m_viewDirty = true;
}

void Library::Camera::UpdateViewMatrix()
{
	if (m_viewDirty)
	{
		// Keep camera's axes orthogonal to each other and of unit length.
		*m_look = DirectX::XMVector3Normalize(*m_look);
		*m_up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(*m_look, *m_right));

		// U, L already ortho-normal, so no need to normalize cross product.
		*m_right = DirectX::XMVector3Cross(*m_up, *m_look);

		// Fill in the view matrix entries.
		float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(*m_position, *m_right));
		float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(*m_position, *m_up));
		float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(*m_position, *m_look));

		*m_view =  DirectX::XMMatrixLookToRH(*m_position, *m_look, *m_up);

		m_viewDirty = false;
	}
}