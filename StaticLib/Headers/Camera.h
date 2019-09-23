#pragma once

#include <memory>
#include <DirectXMath.h>

struct D3D11_VIEWPORT;

namespace Library
{
	class GameTime;
	class Camera
	{
	public:
		enum class MoveDir 
		{ 
			None,
			Forward, 
			Backward, 
			Right, 
			Left, 
			Up, 
			Down 
		};
		Camera(float fov, int width, int height, float nearPlane, float farPlane);
		virtual ~Camera();

		virtual const DirectX::XMMATRIX GetView();
		virtual const DirectX::XMMATRIX GetProjection() const;
		virtual const D3D11_VIEWPORT* GetViewport() const;
		virtual void UpdateViewport();

		DirectX::XMVECTOR GetPosition() const;
		void Move(MoveDir dir);
		void Pitch(float angle);
		void RotateY(float angle);
		void UpdateViewMatrix(const GameTime& gameTime);
	protected:
		virtual void UpdateProjection();	
		virtual void CalculateVeiw();

		DirectX::XMFLOAT4X4 m_view;
		DirectX::XMFLOAT4X4 m_projection;

		std::unique_ptr<D3D11_VIEWPORT> m_viewport;

		DirectX::XMFLOAT3 m_position;
		DirectX::XMFLOAT3 m_look;
		DirectX::XMFLOAT3 m_up;
		DirectX::XMFLOAT3 m_right;
		
		DirectX::XMFLOAT3 m_direction;

		MoveDir m_movedir;

		float m_fov;
		int m_width;
		int m_height;
		float m_near;
		float m_far;
		bool m_viewDirty;
	};
}
