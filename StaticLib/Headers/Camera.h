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

		virtual const DirectX::XMMATRIX* GetView();
		virtual const DirectX::XMMATRIX* GetProjection() const;
		virtual const D3D11_VIEWPORT* GetViewport() const;

		DirectX::XMFLOAT3* GetPosition() const;
		void UpdatePosition(float x, float y, float z);
		void Pitch(float angle);
		void RotateY(float angle);
		void UpdateViewMatrix();
	protected:
		virtual void UpdateProjection();
		virtual void UpdateViewport();

		DirectX::XMFLOAT4X4 m_view;
		std::unique_ptr<DirectX::XMMATRIX> m_viewM;
		std::unique_ptr<DirectX::XMMATRIX> m_projection;
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;
		std::unique_ptr<DirectX::XMFLOAT3> m_position;
		std::unique_ptr<DirectX::XMFLOAT3> m_look;
		std::unique_ptr<DirectX::XMFLOAT3> m_up;
		std::unique_ptr<DirectX::XMFLOAT3> m_right;

		float m_fov;
		int m_width;
		int m_height;
		float m_near;
		float m_far;
		bool m_viewDirty;
	};
}
