#pragma once

#include <memory>
#include <DirectXMath.h>

struct D3D11_VIEWPORT;

namespace Library
{
	class Camera
	{
	public:
		enum class MoveDir 
		{ 
			Forward, 
			Backward, 
			Right, 
			Left, 
			Up, 
			Down 
		};
		Camera(float fov, int width, int height, float nearPlane, float farPlane);
		virtual ~Camera();

		virtual const DirectX::XMMATRIX* GetView();
		virtual const DirectX::XMMATRIX* GetProjection() const;
		virtual const D3D11_VIEWPORT* GetViewport() const;

		DirectX::XMVECTOR* GetPosition() const;
		void Move(MoveDir dir);
		void Pitch(float angle);
		void RotateY(float angle);
		void UpdateViewMatrix();
	protected:
		virtual void UpdateProjection();
		virtual void UpdateViewport();

		std::unique_ptr<DirectX::XMMATRIX> m_view;
		std::unique_ptr<DirectX::XMMATRIX> m_projection;

		std::unique_ptr<D3D11_VIEWPORT> m_viewport;

		std::unique_ptr<DirectX::XMVECTOR> m_position;

		std::unique_ptr<DirectX::XMVECTOR> m_look;
		std::unique_ptr<DirectX::XMVECTOR> m_up;
		std::unique_ptr<DirectX::XMVECTOR> m_right;

		float m_fov;
		int m_width;
		int m_height;
		float m_near;
		float m_far;
		bool m_viewDirty;
	};
}
