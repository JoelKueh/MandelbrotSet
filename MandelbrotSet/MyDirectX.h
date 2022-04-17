#pragma once

#include "framework.h"

// DirectX Header Files
#include <DirectXMath.h>
using namespace DirectX;
#include <d3d11.h>
#include <d3dcompiler.h>
//#include <d3d11_1.h>
//#include <d3d11_2.h>
//#include <d3d11_3.h>
//#include <d3d11_4.h>
//#include <d3d11sdklayers.h>
//#include <d3d11shader.h>
//#include <d3d11shadertracing.h>
//#include <d3dcommon.h>
//#include <d3dcsx.h>
//#include <windows.graphics.directx.direct3d11.interop.h>
#pragma comment (lib, "D3D11.lib")
#pragma comment (lib, "D3Dcompiler.lib")
//#pragma comment (lib, "d3d11_1.lib")
//#pragma comment (lib, "d3d11_2.lib")
//#pragma comment (lib, "d3d11_3.lib")
//#pragma comment (lib, "d3d11_4.lib")
//#pragma comment (lib, "d3d11sdklayers.lib")
//#pragma comment (lib, "d3d11shader.lib")
//#pragma comment (lib, "d3d11shadertracing.lib")
//#pragma comment (lib, "d3dcommon.lib")
//#pragma comment (lib, "d3dcsx.lib")
//#pragma comment (lib, "windows.graphics.directx.direct3d11.interop.lib")

class MyDirectX
{
public:
	MyDirectX();
	
	VOID Draw();
	VOID InitDirectX(HWND hWnd);
	VOID FreeDirectX();

private:

	HRESULT InitDevice(HWND hWnd);
	HRESULT CreateVertexBuffer();
	HRESULT CompileShaders();
	HRESULT CompileComputeShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint,
		_In_ ID3D11Device* device, _Outptr_ ID3DBlob** blob);

	struct simpleVertex
	{
		XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 col = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	struct dxData
	{
		IDXGISwapChain* swapChain = nullptr;
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* deviceContext = nullptr;
		ID3D11ComputeShader* computeShader = nullptr;
		ID3D11RenderTargetView* renderTarget = nullptr;
		ID3D11Texture2D* frameBuffer = nullptr;
		ID3D11Texture2D* tex = nullptr;
		ID3D11ShaderResourceView* shaderResourceView = nullptr;
		ID3D11InputLayout* vertexLayout = nullptr;
		ID3D11Buffer* vertexBuffer = nullptr;

		int width = 0;
		int height = 0;

		simpleVertex vertices[4]
		{
			// Vertex 1
			XMFLOAT3(-3.0f, 3.0f, 0.5f),
			XMFLOAT3(1.0f, 1.0f, 1.0f),

			// Vertex 2
			XMFLOAT3(3.0f, 3.0f, 0.5f),
			XMFLOAT3(1.0f, 1.0f, 1.0f),

			// Vertex 3
			XMFLOAT3(-3.0f, -3.0f, 0.5),
			XMFLOAT3(1.0f, 1.0f, 1.0f),

			// Vertex 4
			XMFLOAT3(3.0f, -3.0f, 0.5f),
			XMFLOAT3(1.0f, 1.0f, 1.0f),
		};
	};
	dxData g_dx;
};

