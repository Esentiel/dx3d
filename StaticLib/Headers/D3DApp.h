#pragma once

#include <memory>
#include <vector>
#include <wrl.h>

#include "GameTime.h"



struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
struct ID3D11Texture2D;

namespace Library
{
	class RenderScene;
	class Mesh;
	class ShaderManager;
	class TextureManager;
	class FileManager;
	class Camera;
	class PostProcessor;
	class ShadowMap;
	class SkyBox;

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
		
		unsigned int m_sampleCount;
		unsigned int m_levels;

		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceCtx;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_rtvs;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsbView;
		
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_backBuffers;

		Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterState;

		std::unique_ptr<RenderScene> m_renderScene;
		std::unique_ptr<ShaderManager> m_shaderManager;
		std::unique_ptr<TextureManager> m_textureManager;
		std::unique_ptr<FileManager> m_fileManager;
		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<PostProcessor> m_postProcessor;
		std::unique_ptr<ShadowMap> m_shadowMap;
		std::unique_ptr<SkyBox> m_skyBox;
	};
}


