#include "MyDirectX.h"

#include <Debugapi.h>
#include <string>

MyDirectX::MyDirectX()
{

}

VOID MyDirectX::Draw()
{
	// Clear the back buffer and set it to white to prepare for the next frame
	float background_colour[4] = {
	  0.0f, 0.0f, 0.0f, 1.0f };
	g_dx.deviceContext->ClearRenderTargetView(
		g_dx.renderTargetView, background_colour);

	// Basically, the dimensions of the frame the is being displayed.
	D3D11_VIEWPORT viewport = {
	  0.0f,
	  0.0f,
	  g_dx.width,
	  g_dx.height,
	  0.0f,
	  1.0f };
	g_dx.deviceContext->RSSetViewports(1, &viewport);

	// Allows us to render to the back buffer rather than the screen.
	g_dx.deviceContext->OMSetRenderTargets(1, &g_dx.renderTargetView, NULL);

	// Feeds in the vertex buffer with the specification that it it a trainglestrip, built in a certain order listed in the documentation.
	g_dx.deviceContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_dx.deviceContext->IASetInputLayout(g_dx.inputLayout);
	g_dx.deviceContext->IASetVertexBuffers(
		0,
		1,
		&g_dx.vertexBuffer,
		&g_dx.vertexStride,
		&g_dx.vertexOffset);

	// Sends in the vertex and pixel shaders.
	g_dx.deviceContext->CSSetShader(g_dx.cs, NULL, 0);
	g_dx.deviceContext->VSSetShader(g_dx.vs, NULL, 0);
	g_dx.deviceContext->PSSetShader(g_dx.ps, NULL, 0);

	// Uses the shaders to draw to the buffer.
	g_dx.deviceContext->Draw(g_dx.vertexCount, 0);

	// Flips the buffer with the image that is on the screen.
	g_dx.swapChain->Present(1, 0);
}

VOID MyDirectX::InitDirectX(HWND hWnd)
{
	InitDevice(hWnd);
	InitTargetView();
	CompileShaders();
	InitCSOutput();
	InitVertexBuffer();
}

VOID MyDirectX::FreeDirectX()
{

}

VOID MyDirectX::InitDevice(HWND hWnd)
{
	// Create a rectangle to contain a couple parameters about the user window
	RECT rect;
	GetClientRect(hWnd, &rect);
	g_dx.width = (float)(rect.right - rect.left);
	g_dx.height = (float)(rect.bottom - rect.top);

	OutputDebugStringA(std::to_string(g_dx.width).c_str());

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = g_dx.width;
	sd.BufferDesc.Height = g_dx.height;
	sd.BufferDesc.RefreshRate.Numerator = 0; // Just render the frame as fast as possible
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Use R8G8B8A8 color format
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = true; // Call IDXGI::SetFullscreenState to change


	// Create an array of featureLevels to be tested in the D3D11CreateDeviceAndSwapChain Function
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Plan for single threading, then tack on a debug flag if it is defined.
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL, // Using the defualt adapter
		D3D_DRIVER_TYPE_HARDWARE, // Using hardware accelleration
		NULL, // Set NULL because we are not using D3D_DRIVER_TYPE_HARDWARE above
		flags, // Use a singlethreaded device because we won't be calling the functions across threads. Then use debug mode if it is installed.
		&featureLevel, // Use the 11.0 feature level version.
		1, // Specify that there is one item in the array of feature levels
		D3D11_SDK_VERSION, // This is what I was told to do by the Windows Documentation
		&sd, // Pass in the swap chain description that we created above
		&g_dx.swapChain, // Pass in a reference to a pointer, so that the function can direct us to a new object.
		&g_dx.device,
		NULL, // We don't need to determine which feature level we created.
		&g_dx.deviceContext
	);
	assert(S_OK == hr && g_dx.swapChain && g_dx.device && g_dx.deviceContext && "Device or Swap Chain Initialization Failed"); // Checking to see if everything worked
}

VOID MyDirectX::InitTargetView()
{
	ID3D11Texture2D* framebuffer;
	HRESULT hr = g_dx.swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer);
	assert(SUCCEEDED(hr) && "Frame Buffer Initialization Failed");

	hr = g_dx.device->CreateRenderTargetView(
		framebuffer, 0, &g_dx.renderTargetView);
	assert(SUCCEEDED(hr) && "Render Target View Initialization Failed");
	framebuffer->Release();
}

VOID MyDirectX::CompileShaders()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
	ID3DBlob* vs_blob_ptr = NULL, * ps_blob_ptr = NULL, * cs_blob_ptr = NULL, * error_blob = NULL;

	// COMPILE VERTEX SHADER
	HRESULT hr = D3DCompileFromFile(
		L"VertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		&vs_blob_ptr,
		&error_blob
	);
	if (FAILED(hr)) {
		if (error_blob) {
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (vs_blob_ptr) { vs_blob_ptr->Release(); }
		assert(false && "Vertex Shader Compilation Failed");
	}

	// COMPILE COMPUTE SHADER
	hr = D3DCompileFromFile(
		L"ComputeShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"cs_5_0",
		flags,
		0,
		&cs_blob_ptr,
		&error_blob
	);
	if (FAILED(hr)) {
		if (error_blob) {
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (vs_blob_ptr) { vs_blob_ptr->Release(); }
		assert(false && "Vertex Shader Compilation Failed");
	}

	// COMPILE PIXEL SHADER
	hr = D3DCompileFromFile(
		L"PixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		flags,
		0,
		&ps_blob_ptr,
		&error_blob
	);
	if (FAILED(hr)) {
		if (error_blob) {
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (ps_blob_ptr) { ps_blob_ptr->Release(); }
		assert(false && "Pixel Shader Compilation Failed");
	}

	hr = g_dx.device->CreateComputeShader(
		cs_blob_ptr->GetBufferPointer(),
		cs_blob_ptr->GetBufferSize(),
		NULL,
		&g_dx.cs
	);
	assert(SUCCEEDED(hr) && "Compute Shader Creation Failed");

	hr = g_dx.device->CreateVertexShader(
		vs_blob_ptr->GetBufferPointer(),
		vs_blob_ptr->GetBufferSize(),
		NULL,
		&g_dx.vs
	);
	assert(SUCCEEDED(hr) && "Vertex Shader Creation Failed");

	hr = g_dx.device->CreatePixelShader(
		ps_blob_ptr->GetBufferPointer(),
		ps_blob_ptr->GetBufferSize(),
		NULL,
		&g_dx.ps
	);
	assert(SUCCEEDED(hr) && "Pixel Shader Creation Failed");

	// InputLayoutCreation - Uses vs_blob_ptr from above
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = g_dx.device->CreateInputLayout(
		inputElementDesc,
		ARRAYSIZE(inputElementDesc),
		vs_blob_ptr->GetBufferPointer(),
		vs_blob_ptr->GetBufferSize(),
		&g_dx.inputLayout
	);
	assert(SUCCEEDED(hr) && "Input Layout Creation Failed");
}

VOID MyDirectX::InitVertexBuffer()
{
	/*** load mesh data into vertex buffer **/
	D3D11_BUFFER_DESC vertex_buff_descr = {};
	vertex_buff_descr.ByteWidth = sizeof(g_dx.vertexArray);
	vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
	vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sr_data = { 0 };
	sr_data.pSysMem = g_dx.vertexArray;
	HRESULT hr = g_dx.device->CreateBuffer(
		&vertex_buff_descr,
		&sr_data,
		&g_dx.vertexBuffer);
	assert(SUCCEEDED(hr) && "Vertex Buffer Creation Failed");
}

VOID MyDirectX::InitCSOutput()
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = g_dx.width;
	texDesc.Height = g_dx.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// TEST THIS FOR SOME QUALITY STUFF
	texDesc.SampleDesc.Count = 2;
	texDesc.SampleDesc.Quality = 2;

	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;

	HRESULT hr = g_dx.device->CreateTexture2D(
		&texDesc,
		NULL,
		&g_dx.csOutput);
	assert(SUCCEEDED(hr) && "CSOutput Creation Failed");
}

VOID MyDirectX::InitCSInput()
{
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
	constBufferDesc.ByteWidth = sizeof(double);
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HRESULT hr = g_dx.device->CreateBuffer(
		&constBufferDesc,
		NULL,
		&g_dx.csInput);
	assert(SUCCEEDED(hr) && "CSInput Creation Failed");
}