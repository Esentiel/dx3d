#pragma once

#include <wrl.h>
#include <string>

struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11RasterizerState;

namespace Library
{
	class PostProcessor
	{
	public:
		PostProcessor(int width, int height);
		~PostProcessor();

		void Initialize();
		void Draw();
		void Begin();

		ID3D11ShaderResourceView* GetShaderRes();
		ID3D11RenderTargetView* GetOffscreenRtv();
		ID3D11DepthStencilView* GetOffscreenDsb();

		PostProcessor(const PostProcessor& rhs) = delete;
		PostProcessor& operator=(const PostProcessor& rhs) = delete;
	private:
		void CreateInputLayout();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateRasterState();
		
		Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterState;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderRes;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_offscreenRtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_offscreenDsb;
		
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

		std::string m_vertexShaderName;
		std::string m_pixelShaderName;
	};
}
