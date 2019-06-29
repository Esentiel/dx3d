#pragma once

#include <memory>
#include <DirectXMath.h>

struct D3D11_VIEWPORT;

namespace Library
{
	class Camera
	{
	public:
		Camera(float fov, int width, int height, float nearPlane, float farPlane);
		virtual ~Camera();

		virtual const DirectX::XMMATRIX* GetView() const;
		virtual const DirectX::XMMATRIX* GetProjection() const;
		virtual const D3D11_VIEWPORT* GetViewport() const;

	protected:
		virtual void UpdateView();
		virtual void UpdateProjection();
		virtual void UpdateViewport();

		std::unique_ptr<DirectX::XMMATRIX> m_view;
		std::unique_ptr<DirectX::XMMATRIX> m_projection;
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;

		float m_fov;
		int m_width;
		int m_height;
		float m_near;
		float m_far;

	};
}
