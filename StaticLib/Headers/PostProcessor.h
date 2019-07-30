#pragma once

#include <wrl.h>

struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class PostProcessor
{
public:
	PostProcessor(int width, int height);
	~PostProcessor();

	ID3D11ShaderResourceView* GetShaderRes();
	ID3D11RenderTargetView* GetOffscreenRtv();
	ID3D11DepthStencilView* GetOffscreenDsb();

	void Start();

	PostProcessor(const PostProcessor& rhs) = delete;
	PostProcessor& operator=(const PostProcessor& rhs) = delete;
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderRes;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_offscreenRtv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_offscreenDsb;
};

