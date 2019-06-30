#pragma once

#include <memory>

namespace DirectX
{
	struct XMMATRIX;
	struct XMFLOAT3;
}

namespace Library
{
	class Transformations
	{
	public:
		Transformations();
		~Transformations();
		DirectX::XMMATRIX* GetModel();

		void Move(const DirectX::XMFLOAT3 &direction);
		void Rotate(const DirectX::XMFLOAT3 &rotation);
		void Scale(const DirectX::XMFLOAT3 &scale);

	private:
		std::unique_ptr<DirectX::XMMATRIX> m_model;
		std::unique_ptr<DirectX::XMFLOAT3> m_position;
		std::unique_ptr<DirectX::XMFLOAT3> m_rotation;
		std::unique_ptr<DirectX::XMFLOAT3> m_scale;
	};
}

