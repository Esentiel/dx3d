#pragma once

#include <memory>
#include <wrl.h>

#include "GameTime.h"



struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;

namespace Library
{
	class RenderScene;
	class Mesh;
	class ShaderManager;
	class Camera;
	struct GD3DApp;

	class D3DApp
	{
	public:
		D3DApp(HWND hwnd, unsigned int width, unsigned int height);
		~D3DApp();

		void Initialize();
		void Draw(const GameTime &gameTime);

	private:
		void DrawMesh(Mesh* mesh);

		std::unique_ptr<GD3DApp> m_globalApp;

		HWND m_hwnd;
		unsigned int m_width;
		unsigned int m_height;

		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceCtx;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsbView;
		

		Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterState;

		std::unique_ptr<RenderScene> m_renderScene;
		std::unique_ptr<ShaderManager> m_shaderManager;
		std::unique_ptr<Camera> m_camera;
	};
}


