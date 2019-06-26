#pragma once

#include <memory>
#include <wrl.h>

#include "GameTime.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGIFactory2;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct D3D11_VIEWPORT;

namespace Library
{
	class D3DApp
	{
	public:
		D3DApp(HWND hwnd);
		~D3DApp();

		void Initialize();
		void Draw(const GameTime &gameTime);

	private:
		HWND m_hwnd;
		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceCtx;
		Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsbView;
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;
	};
}


