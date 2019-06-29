#include "stdafx.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Library;

Camera::Camera(float fov, int width, int height, float nearPlane, float farPlane) :
	m_fov(fov),
	m_width(width),
	m_height(height),
	m_near(nearPlane),
	m_far(farPlane),
	m_viewport(new D3D11_VIEWPORT)
{
	UpdateView();
	UpdateProjection();
	UpdateViewport();
}

Camera::~Camera()
{
}

const DirectX::XMMATRIX* Camera::GetView() const
{
	return m_view.get();
}

const DirectX::XMMATRIX* Camera::GetProjection() const
{
	return m_projection.get();
}

void Camera::UpdateView()
{
	DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.f, 10.0f, 1.0f);
	DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_view.reset(new DirectX::XMMATRIX(DirectX::XMMatrixLookToRH(eye, at, up)));
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
