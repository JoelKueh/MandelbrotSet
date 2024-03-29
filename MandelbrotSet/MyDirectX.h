#pragma once

#include "framework.h"

// DirectX Header Files
#include <DirectXMath.h>
using namespace DirectX;
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <dxgi.h>
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

#pragma comment (lib, "user32")
#pragma comment (lib, "D3D11.lib")
#pragma comment (lib, "D3Dcompiler.lib")
#pragma comment (lib, "dxgi.lib")
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

	VOID ResizeDevices(HWND hWnd);

private:

	__declspec(align(32)) struct ConstBufferData {
		float centerX;
		float centerY;
		float width;
		float windWidth;
		float windHeight;
		float ssLvl;
	};

	UINT ssLvl = 4;
	UINT syncCounter = 0;

	struct dxData
	{
		ID3D11Device5* device = NULL;
		ID3D11DeviceContext4* deviceContext = NULL;
		IDXGISwapChain* swapChain = NULL;

		ID3D11RenderTargetView* renderTargetView = NULL;
		ID3D11InputLayout* inputLayout = NULL;

		ID3D11Texture2D* csOutput = NULL;
		ID3D11UnorderedAccessView* csOutputViewRW = NULL;
		ID3D11ShaderResourceView* csOutputViewR = NULL;
		ID3D11Buffer* csInput = NULL;
		ID3D11Fence* fence = NULL;

		ID3D11ComputeShader* cs = NULL;
		ID3D11VertexShader* vs = NULL;
		ID3D11PixelShader* ps = NULL;

		ID3D11Buffer* vertexBuffer = NULL;
		float vertexArray[12] = {
		   -1.0f, -1.0f,  0.0f, // point at bottom-left
		   -1.0f,  1.0f,  0.0f, // point at top-left
			1.0f, -1.0f,  0.0f, // point at bottom-right
			1.0f,  1.0f,  0.0f,  // point at top-right
		};
		UINT vertexStride = 3 * sizeof(float);
		UINT vertexOffset = 0;
		UINT vertexCount = 4;

		UINT width = 0;
		UINT height = 0;

		ConstBufferData constBufferCPU;
	};
	dxData g_dx;

	VOID InitDevice(HWND hWnd);
	VOID InitTargetView();
	VOID CompileShaders();
	VOID InitVertexBuffer();
	VOID InitCSOutputResource();
	VOID InitCSOutputView();
	VOID InitPSInputView();
	VOID InitCSInput();
	VOID InitFence();

	VOID FlushConstantBuffer();
	VOID RecreateCSOutput();
};