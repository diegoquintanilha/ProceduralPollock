#pragma once

#include <Windows.h>
#include <d3d11.h>

class Graphics
{
public:
	Graphics(int width, int height, const char* shaderPtr, int shaderSize);
	~Graphics();

	bool UpdateInputs() const;
	void CreatePixelShader(const void* shaderPtr, int shaderSize);
	void UpdateConstantBuffer(float x, float y, float z, float w) const;
	void DrawViewportQuad() const;
	void SwapBuffers() const;

private:
	// Window
	WNDCLASSEX m_WindowClass = {};
	HWND m_WindowHandle = nullptr;

	// DirectX11
	IDXGISwapChain* m_SwapChain = nullptr;
	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	ID3D11Buffer* m_ConstantBuffer = nullptr;

	void CreateQuad();
	void CreateBlendingMode();
	void CreateIndexBuffer();
	void CreateConstantBuffer();
	void CreateVertexShader();

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

