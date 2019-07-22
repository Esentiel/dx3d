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
	m_positionV(new DirectX::XMVECTOR(DirectX::XMVectorSet(.0f, 30.f, 40.0f, 1.f))),
	m_lookV(new DirectX::XMVECTOR(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.f))),
	m_upV(new DirectX::XMVECTOR(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.f))),
	m_rightV(new DirectX::XMVECTOR(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.f))),
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
	return m_positionV.get();
}

void Library::Camera::Move(MoveDir dir)
{
	DirectX::XMFLOAT3 dirN;

	switch (dir)
	{
	case MoveDir::Forward:
		//dirN = *m_look;
		break;
	case MoveDir::Backward:
		//dirN = DirectX::XMVectorNegate(DirectX::)
		break;
	}

	/*if (y)
	{
		if (y > 0)
			dirN = DirectX::XMVector4Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(m_look.get()), DirectX::XMLoadFloat3(m_position.get())));
		else
			dirN = DirectX::XMVector4Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(m_look.get()), DirectX::XMLoadFloat3(m_position.get())));
	}*/

	

	/*DirectX::XMFLOAT4 normalDir4;
	DirectX::XMStoreFloat4(&normalDir4, dirN);

	m_position->x += (normalDir4.x * x);
	m_position->y += (normalDir4.y * y);
	m_position->z += (normalDir4.z * z);
	
	m_viewDirty = true;*/
}

void Library::Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	DirectX::XMMATRIX R = DirectX::XMMatrixRotationAxis(*m_rightV, angle);

	*m_upV = DirectX::XMVector3TransformNormal(*m_upV, R);
	*m_lookV = DirectX::XMVector3TransformNormal(*m_lookV, R);

	m_viewDirty = true;
}

void Library::Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(angle);

	*m_rightV = DirectX::XMVector3TransformNormal(*m_rightV, R);
	*m_lookV =  DirectX::XMVector3TransformNormal(*m_lookV, R);

	m_viewDirty = true;
}

void Library::Camera::UpdateViewMatrix()
{
	if (m_viewDirty)
	{
		// Keep camera's axes orthogonal to each other and of unit length.
		*m_lookV = DirectX::XMVector3Normalize(*m_lookV);
		*m_upV = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(*m_lookV, *m_rightV));

		// U, L already ortho-normal, so no need to normalize cross product.
		*m_rightV = DirectX::XMVector3Cross(*m_upV, *m_lookV);

		// Fill in the view matrix entries.
		float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(*m_positionV, *m_rightV));
		float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(*m_positionV, *m_upV));
		float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(*m_positionV, *m_lookV));

		*m_view =  DirectX::XMMatrixLookToRH(*m_positionV, *m_lookV, *m_upV);

		m_viewDirty = false;
	}
}