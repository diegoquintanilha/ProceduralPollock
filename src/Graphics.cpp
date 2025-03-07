#include "Graphics.h"

#include <iostream>
#include <string>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma region Defines

#define SCOPE(scope) do scope while (false)

#define LOG(x) std::cout << x << std::endl

#define _ERROR_AT "Error on file '" << __FILE__ << "', on function '" << __FUNCSIG__ << "' at line " << __LINE__ << ": "

#define _PAUSE system("pause")

#define _ERROR(errorMessage)						\
	SCOPE											\
	({												\
		LOG(_ERROR_AT << errorMessage << ".");	\
		_PAUSE;										\
	})

#define ASSERT(value, errorMessage)					\
	SCOPE											\
	({												\
		if (!(value))								\
			_ERROR(errorMessage);					\
	})

#define ASSERT_CALL(function, errorMessage)			\
	SCOPE											\
	({												\
		if (!(function))							\
			_ERROR(errorMessage);					\
	})

#define ASSERT_WINDOWS(function, errorMessage)		\
	SCOPE											\
	({												\
		if (FAILED(function))						\
			_ERROR(errorMessage);					\
	})

#define RELEASE_COM_PTR(p) SCOPE({if (p) { p->Release(); p = nullptr; }})

#pragma endregion

Graphics::Graphics(int width, int height, const char* shaderPtr, int shaderSize)
{
	HINSTANCE hInstance = GetModuleHandleW(nullptr);

	static const wchar_t* className = L"WindowClass";

	// Create window class
	m_WindowClass =
	{
		.cbSize			= sizeof(WNDCLASSEX),
		.style			= CS_OWNDC,
		.lpfnWndProc	= WindowProc,
		.cbClsExtra		= 0,
		.cbWndExtra		= 0,
		.hInstance		= hInstance,
		.hIcon			= nullptr,
		.hCursor		= LoadCursorW(nullptr, IDC_ARROW), // Use standard cursor
		.hbrBackground	= nullptr, // Do not use winapi to draw anything, we will draw ourselves
		.lpszMenuName	= nullptr,
		.lpszClassName	= className,
		.hIconSm		= nullptr // This uses the same icon for small (TODO: check if this is necessary when using different images on the same .ico file)
	};

	ASSERT_CALL(RegisterClassExW(&m_WindowClass), "could not register window class");

	RECT wr =
	{
		.left = 0,
		.top = 0,
		.right = width,
		.bottom = height
	};
	ASSERT_CALL(AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false), "could not calculate window rect");

	// Create window
	ASSERT_CALL(m_WindowHandle = CreateWindowExW
	(
		0,
		className,
		L"Random Image",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		nullptr,
		nullptr,
		m_WindowClass.hInstance,
		nullptr
	), "could not create window");

	ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);

	// Swap chain descriptor
	DXGI_SWAP_CHAIN_DESC scd =
	{
		.BufferDesc = 
		{
			.Width				= 0,
			.Height				= 0,
			.RefreshRate		= { .Numerator = 60, .Denominator = 1 },
			.Format				= DXGI_FORMAT_R8G8B8A8_UNORM,				// Display (pixel) format descriptor
			.ScanlineOrdering	= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,		// Do not specify the order/direction of the rasterization
			.Scaling			= DXGI_MODE_SCALING_UNSPECIFIED				// Maybe different for fullscreen in smaller resolutions
		},
		.SampleDesc		= { .Count = 1, .Quality = 0 },		// No anti aliasing
		.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT,	// Set the screen buffers as render targets
		.BufferCount	= 1,								// 1 for double buffering, 2 for triple buffering, and so on
		.OutputWindow	= m_WindowHandle,					// Window pointer
		.Windowed		= true,
		.SwapEffect		= DXGI_SWAP_EFFECT_DISCARD,
		.Flags			= 0
	};

	// Initialize DirectX, and get the three main pointers: device, device context and swap chain
	ASSERT_WINDOWS(D3D11CreateDeviceAndSwapChain
	(
		nullptr, // Use default system graphics card
		D3D_DRIVER_TYPE_HARDWARE, // Select hardware graphics acceleration, as opposed to software emulated
		nullptr, // If graphics were to be software emulated, this would be a handle to its DLL (and null otherwise)
		0,
		nullptr, // List of DirectX backwards compatible versions (feature levels)
		0, // Size of previous array
		D3D11_SDK_VERSION,
		&scd,
		&m_SwapChain,
		&m_Device,
		nullptr, // If other feature levels were supported, the current one would be attributed to this pointer
		&m_Context
	),
	"could not create device and/or swap chain");

	// Get a view on the back buffer and set it as the render target
	ID3D11Resource* backBufferResource = nullptr;
	ID3D11RenderTargetView* backBuffer = nullptr;
	ASSERT_WINDOWS(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)(&backBufferResource)), "could not get back buffer");
	ASSERT_WINDOWS(m_Device->CreateRenderTargetView(backBufferResource, nullptr, &backBuffer), "could not create render target view on back buffer");
	m_Context->OMSetRenderTargets(1, &backBuffer, nullptr);
	backBufferResource->Release();

	// Set viewport as the entire window
	D3D11_VIEWPORT vp =
	{
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = float(width),
		.Height = float(height),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};
	m_Context->RSSetViewports(1, &vp);

	// Set primitive topology to triangle strip
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Create a quad that covers the whole window
	CreateQuad();
	
	// Setup the rest of the graphics pipeline
	CreateBlendingMode();
	CreateIndexBuffer();
	CreateConstantBuffer();
	CreateVertexShader();
	CreatePixelShader(shaderPtr, shaderSize);
}
Graphics::~Graphics()	
{
	// DirectX11
	RELEASE_COM_PTR(m_ConstantBuffer);
	RELEASE_COM_PTR(m_Context);
	RELEASE_COM_PTR(m_Device);
	RELEASE_COM_PTR(m_SwapChain);
	
	// Window
	ASSERT_CALL(DestroyWindow(m_WindowHandle), "could not destroy window");
	ASSERT_CALL(UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance), "could not unregister window class");
}

bool Graphics::UpdateInputs() const
{
	MSG msg;
	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return true;
}
void Graphics::CreatePixelShader(const void* shaderPtr, int shaderSize)
{
	#if _DEBUG

		// Compilation flags
		uint32_t flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

		// Compile shader
		ID3DBlob* blob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompile(shaderPtr, shaderSize, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &blob, &errorBlob);

		// Error handling
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				_ERROR((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			else
			{
				_ERROR("Unknown pixel shader compilation error");
			}
		}

	#else

		// Compilation flags
		uint32_t flags = D3DCOMPILE_PARTIAL_PRECISION | D3DCOMPILE_OPTIMIZATION_LEVEL3;

		// Compile shader
		ID3DBlob* blob = nullptr;
		D3DCompile(shaderPtr, shaderSize, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &blob, nullptr);

	#endif

	// Create pixel shader
	ID3D11PixelShader* pixelShader = nullptr;
	m_Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader);
	
	// Bind pixel shader
	m_Context->PSSetShader(pixelShader, nullptr, 0);

	// Release data blob COM pointer
	blob->Release();
	pixelShader->Release();
}
void Graphics::UpdateConstantBuffer(float x, float y, float z, float w) const
{
	const float newData[] = { x, y, z, w };
	// Struct that will hold a pointer to the GPU buffer that will be updated
	D3D11_MAPPED_SUBRESOURCE msr = {};
	// Disable GPU access to the constant buffer data, and get a pointer to it
	ASSERT_WINDOWS(m_Context->Map(m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr), "could not map constant buffer");
	// Update constant buffer through the subresource pointer
	std::memcpy(msr.pData, newData, 4 * sizeof(float));
	// Reenable GPU access to the constant buffer data
	m_Context->Unmap(m_ConstantBuffer, 0);
}
void Graphics::DrawViewportQuad() const
{
	m_Context->DrawIndexed(4, 0, 0);
}
void Graphics::SwapBuffers() const
{
	#if _DEBUG

	HRESULT hr = m_SwapChain->Present(1, 0);

	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			_ERROR("could not swap buffers (device lost)");
		}
		else
		{
			_ERROR("could not swap buffers");
		}
	}

	#else

	m_SwapChain->Present(1, 0);

	#endif
}

void Graphics::CreateQuad()
{
	// Quad vertex coordinates
	float quad[8] =
	{
		-1.0f,  1.0f, // TL
		 1.0f,  1.0f, // TR
		-1.0f, -1.0f, // BL
		 1.0f, -1.0f, // BR
	};

	// Vertex buffer descriptor
	D3D11_BUFFER_DESC vbd =
	{
		.ByteWidth				= 8 * sizeof(float),		// Size of buffer is number of quads * quad byte size (vertices)
		.Usage					= D3D11_USAGE_IMMUTABLE,	// The data that this buffer describe will not change
		.BindFlags				= D3D11_BIND_VERTEX_BUFFER, // This is a vertex buffer descriptor
		.CPUAccessFlags			= 0,
		.MiscFlags				= 0,
		.StructureByteStride	= 2 * sizeof(float)			// Byte size of each vertex
	};

	// Vertex buffer subresource data struct
	D3D11_SUBRESOURCE_DATA vsd = { .pSysMem = &quad };

	// Create vertex buffer
	ID3D11Buffer* vertexBuffer = nullptr;
	ASSERT_WINDOWS(m_Device->CreateBuffer(&vbd, &vsd, &vertexBuffer), "could not create vertex buffer");

	// Bind vertex buffer
	const uint32_t offset = 0;
	m_Context->IASetVertexBuffers(0, 1, &vertexBuffer, &vbd.StructureByteStride, &offset);

	// Release vertex buffer COM pointer
	vertexBuffer->Release();
}
void Graphics::CreateBlendingMode()
{
	// Regular blending descriptor
	D3D11_BLEND_DESC bd = {};
	bd.RenderTarget[0] =
	{
		.BlendEnable = false,
		.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
	};

	// Create regular blending
	ID3D11BlendState* blending = nullptr;
	ASSERT_WINDOWS(m_Device->CreateBlendState(&bd, &blending), "could not create regular blend state");

	// Bind blending state
	m_Context->OMSetBlendState(blending, nullptr, 0xffffffffU);
	
	// Release COM pointer
	blending->Release();
}
void Graphics::CreateIndexBuffer()
{
	uint8_t indices[4] = { 0, 1, 2, 3 };

	// Index buffer descriptor
	D3D11_BUFFER_DESC ibd =
	{
		.ByteWidth				= 4 * sizeof(uint8_t), // Size of buffer is number of quads * quad byte size (indices)
		.Usage					= D3D11_USAGE_IMMUTABLE, // This will not change
		.BindFlags				= D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags			= 0,
		.MiscFlags				= 0,
		.StructureByteStride	= sizeof(uint8_t) // Byte size of each index
	};

	// Index buffer subresource data struct
	D3D11_SUBRESOURCE_DATA isd = { .pSysMem = indices };

	// Create index buffer
	ID3D11Buffer* indexBuffer = nullptr;
	ASSERT_WINDOWS(m_Device->CreateBuffer(&ibd, &isd, &indexBuffer), "could not create index buffer");

	// Bind index buffer
	m_Context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R8_UINT, 0);

	// Release index buffer COM pointer
	indexBuffer->Release();
}
void Graphics::CreateConstantBuffer()
{
	// Constant buffer data
	static constexpr float data[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Constant buffer descriptor
	D3D11_BUFFER_DESC cbd =
	{
		.ByteWidth				= 4 * sizeof(float), // ByteWidth must be a multiple of 16 (in this case, at least exactly 16)
		.Usage					= D3D11_USAGE_DYNAMIC, // This will change every frame (probably more than once)
		.BindFlags				= D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE,
		.MiscFlags				= 0,
		.StructureByteStride	= 0 // Constant buffer is not an array, therefore standard stride value is 0
	};

	// Constant buffer subresource data struct
	D3D11_SUBRESOURCE_DATA csd = { .pSysMem = data };

	// Create constant buffer
	ASSERT_WINDOWS(m_Device->CreateBuffer(&cbd, &csd, &m_ConstantBuffer), "could not create constant buffer");

	// Bind constant buffer to pixel shader
	m_Context->PSSetConstantBuffers(0, 1, &m_ConstantBuffer);
}
void Graphics::CreateVertexShader()
{
	const char shaderStr[] =
		R"(
			struct OutInfo
			{
				float2 uv : TEXCOORD;
				float4 pos : SV_POSITION; // Necessary output for the rasterizer, with this specific format and semantic.
			};
			
			OutInfo main(float2 position : POSITON)
			{
				OutInfo o;
				o.pos = float4(position, 0.0f, 1.0f);
				o.uv = 0.5f * position + 0.5f; // Transform coordinates from (-1.0f, 1.0f) to (0.0f, 1.0f)
				return o;
			}
		)";

	#if _DEBUG

		// Compilation flags
		uint32_t flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;

		// Compile shader
		ID3DBlob* blob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompile(shaderStr, sizeof(shaderStr), nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &blob, &errorBlob);

		// Error handling
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				_ERROR((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			else
			{
				_ERROR("unknown vertex shader compilation error");
			}
		}

	#else

		// Compilation flags
		uint32_t flags = D3DCOMPILE_PARTIAL_PRECISION | D3DCOMPILE_OPTIMIZATION_LEVEL3;

		// Compile shader
		ID3DBlob* blob = nullptr;
		D3DCompile(shaderStr, sizeof(shaderStr), nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &blob, nullptr);

	#endif

	// Create vertex shader
	ID3D11VertexShader* vertexShader = nullptr;
	m_Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader);

	// Bind vertex shader
	m_Context->VSSetShader(vertexShader, nullptr, 0);

	// Vertex input layout descriptor (array with only one entry)
	const D3D11_INPUT_ELEMENT_DESC vld[] =
	{{
		.SemanticName			= "POSITON", // Semantic must match the input of the shader 'main' function
		.SemanticIndex			= 0,
		.Format					= DXGI_FORMAT_R32G32_FLOAT, // Format must match the data type (float2) from the input of the shader 'main' function
		.InputSlot				= 0,
		.AlignedByteOffset		= 0,
		.InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate	= 0
	}};

	// Create vertex input layout
	ID3D11InputLayout* inputLayout;
	ASSERT_WINDOWS(m_Device->CreateInputLayout(vld, 1, blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout), "could not create vertex input layout");

	// Bind vertex input layout
	m_Context->IASetInputLayout(inputLayout);

	// Release COM pointers
	blob->Release();
	vertexShader->Release();
	inputLayout->Release();
}

LRESULT CALLBACK Graphics::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

