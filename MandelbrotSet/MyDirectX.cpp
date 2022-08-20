#include "MyDirectX.h"

#include <Debugapi.h>
#include <string>

MyDirectX::MyDirectX() = default;

VOID MyDirectX::Draw()
{
	// Clear the back buffer and set it to black to prepare for the next frame
	float background_colour[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
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

	// Prep the Compute Shader
	g_dx.deviceContext->CSSetShader(g_dx.cs, NULL, 0);
	g_dx.deviceContext->CSSetConstantBuffers(0, 1, &g_dx.csInput);
	g_dx.deviceContext->CSSetUnorderedAccessViews(0, 1, &g_dx.csOutputViewRW, 0);

	g_dx.deviceContext->Dispatch(g_dx.width / 16 * ssLvl + 16, g_dx.height / 16 * ssLvl + 16, 1);

	// Debind the UAV from the compute shader
	ID3D11UnorderedAccessView* nullUAV = { nullptr };
	g_dx.deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);

	// Send in the Vertex and pixel shaders to finish the job.
	g_dx.deviceContext->VSSetShader(g_dx.vs, NULL, 0);

	g_dx.deviceContext->PSSetShader(g_dx.ps, NULL, 0);
	g_dx.deviceContext->PSSetShaderResources(0, 1, &g_dx.csOutputViewR);
	g_dx.deviceContext->PSSetConstantBuffers(0, 1, &g_dx.csInput);


	// Uses the shaders to draw to the buffer.
	g_dx.deviceContext->Draw(g_dx.vertexCount, 0);

	// Flips the buffer with the image that is on the screen.
	g_dx.swapChain->Present(1, 0);
	ID3D11ShaderResourceView* nullSRV = { nullptr };
	g_dx.deviceContext->PSSetShaderResources(0, 1, &nullSRV);
}

VOID MyDirectX::InitDirectX(HWND hWnd)
{
	InitDevice(hWnd);
	InitTargetView();
	CompileShaders();
	InitCSInput();
	InitCSOutputResource();
	InitCSOutputView();
	InitPSInputView();
	InitVertexBuffer();
	InitFence();
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

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
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
		&device,
		NULL, // We don't need to determine which feature level we created.
		&deviceContext
	);
	assert(S_OK == hr && g_dx.swapChain && device && deviceContext && "Device or Swap Chain Initialization Failed"); // Checking to see if everything worked
	hr = device->QueryInterface(__uuidof(ID3D11Device5), (void**)&g_dx.device); // Capture a pointer to a Device5 version of the device created earlier
	assert(SUCCEEDED(hr) && "Device Update to Device 5 failed"); // TODO: Implement fallback.
	hr = deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)&g_dx.deviceContext);
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
	if (FAILED(hr))
	{
		if (error_blob)
		{
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (vs_blob_ptr) { vs_blob_ptr->Release(); }
		assert(false && "Compute Shader Compilation Failed");
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
	if (FAILED(hr))
	{
		if (error_blob)
		{
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
	D3D11_BUFFER_DESC vertex_buff_descr;
	ZeroMemory(&vertex_buff_descr, sizeof(vertex_buff_descr));
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

VOID MyDirectX::InitCSOutputResource()
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = g_dx.width * ssLvl;
	texDesc.Height = g_dx.height * ssLvl;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Only sample once, becasue UAV's don't support multisampling.
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;

	HRESULT hr = g_dx.device->CreateTexture2D(
		&texDesc,
		NULL,
		&g_dx.csOutput);
	assert(SUCCEEDED(hr) && "CSOutput Creation Failed");
}

VOID MyDirectX::InitCSOutputView() {
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

	HRESULT hr = g_dx.device->CreateUnorderedAccessView(
		g_dx.csOutput,
		&uavDesc,
		&g_dx.csOutputViewRW
	);
	assert(SUCCEEDED(hr) && "CSOutputViewRW Creation Failed");
}

VOID MyDirectX::InitPSInputView() {
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr = g_dx.device->CreateShaderResourceView(
		g_dx.csOutput,
		&srvDesc,
		&g_dx.csOutputViewR
	);
	assert(SUCCEEDED(hr) && "CSOutputViewR Creation Failed");
}

VOID MyDirectX::InitCSInput()
{
	g_dx.constBufferCPU.centerX = -.5;
	g_dx.constBufferCPU.centerY = 0;
	g_dx.constBufferCPU.width = 5.0;
	g_dx.constBufferCPU.windWidth = g_dx.width;
	g_dx.constBufferCPU.windHeight = g_dx.height;
	g_dx.constBufferCPU.ssLvl = ssLvl;

	D3D11_SUBRESOURCE_DATA cBuffInitData;
	ZeroMemory(&cBuffInitData, sizeof(cBuffInitData));
	cBuffInitData.pSysMem = &g_dx.constBufferCPU;

	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
	constBufferDesc.ByteWidth = 32;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // TODO: Learn about changing this to DYNAMIC
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.MiscFlags = 0;
	constBufferDesc.StructureByteStride = 4;

	HRESULT hr = g_dx.device->CreateBuffer(
		&constBufferDesc,
		&cBuffInitData,
		&g_dx.csInput
	);
	assert(SUCCEEDED(hr) && "CSInput Creation Failed");
}

VOID MyDirectX::InitFence()
{
	HRESULT hr = g_dx.device->CreateFence(
		0,
		D3D11_FENCE_FLAG_NONE,
		__uuidof(ID3D11Fence),
		reinterpret_cast<void**>(&g_dx.fence)
	);
	assert(SUCCEEDED(hr) && "Fence Creation Failed");
}

VOID MyDirectX::ResizeDevices(HWND hWnd)
{
	// Update the client rectangle
	RECT rect;
	GetClientRect(hWnd, &rect);
	g_dx.width = (float)(rect.right - rect.left);
	g_dx.height = (float)(rect.bottom - rect.top);

	g_dx.deviceContext->OMSetRenderTargets(0, 0, 0);

	// Release all outstanding references to the swap chain's buffers.
	g_dx.renderTargetView->Release();

	HRESULT hr;
	// Preserve the existing buffer count and format.
	// Automatically choose the width and height to match the client rect for HWNDs.
	hr = g_dx.swapChain->ResizeBuffers(1, g_dx.width, g_dx.height, DXGI_FORMAT_UNKNOWN, 0);

	// Perform error handling here!

	// Get buffer and create a render-target-view.
	ID3D11Texture2D* frameBuffer;
	hr = g_dx.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(void**)&frameBuffer);
	assert(SUCCEEDED(hr) && "Render Target View Frame Buffer Remake Failed");

	if (frameBuffer == 0)
	{
		assert(false && "Frame Buffer Creation Failed");
		return;
	}

	hr = g_dx.device->CreateRenderTargetView(frameBuffer, NULL,
		&g_dx.renderTargetView);
	assert(SUCCEEDED(hr) && "Render Target View Remake Failed");
	frameBuffer->Release();

	g_dx.deviceContext->OMSetRenderTargets(1, &g_dx.renderTargetView, NULL);

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = g_dx.width;
	vp.Height = g_dx.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_dx.deviceContext->RSSetViewports(1, &vp);

#if defined( DEBUG ) || defined( _DEBUG )
	// DEBUG
	char outputWidth[20];
	sprintf_s(outputWidth, "Width: %d\n", g_dx.width);
	char outputHeight[20];
	sprintf_s(outputHeight, "Height: %d\n", g_dx.height);
	OutputDebugStringA(outputWidth);
	OutputDebugStringA(outputHeight);
#endif

	g_dx.constBufferCPU.windWidth = g_dx.width;
	g_dx.constBufferCPU.windHeight = g_dx.height;
	FlushConstantBuffer();

	RecreateCSOutput();
}

VOID MyDirectX::FlushConstantBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedSubr;
	ZeroMemory(&mappedSubr, sizeof(mappedSubr));

	g_dx.deviceContext->Map(g_dx.csInput, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubr);
	memcpy(mappedSubr.pData, &g_dx.constBufferCPU, sizeof(g_dx.constBufferCPU));
	g_dx.deviceContext->Unmap(g_dx.csInput, 0);
}

VOID MyDirectX::RecreateCSOutput()
{
	g_dx.csOutputViewRW->Release();
	g_dx.csOutputViewR->Release();
	g_dx.csOutput->Release();

	InitCSOutputResource();
	InitCSOutputView();
	InitPSInputView();
}