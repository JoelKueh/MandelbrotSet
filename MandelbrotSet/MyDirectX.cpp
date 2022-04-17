#include "MyDirectX.h"

MyDirectX::MyDirectX()
{

}

VOID MyDirectX::Draw()
{
// Input-Assembler Stage.
    // No Buffer is made because we are only dealing with one possible rendering face, a triangle-strip / rectangle with vertices on the four corners of the screen.


// Vertex Shader Stage

// Compute Shatder Stage, writes to a texture that can hopefully be accessed later when we need to map it to our vertices.
// Remove this in place of a pixel shader that directly calculates pixel color, rather than using this texture to map to the pixels.
    //g_dx.deviceContext->CSSetShader(g_dx.computeShader, nullptr, 0);
    //g_dx.deviceContext->CSSetShaderResources(0, 1, &g_dx.shaderResourceView);
    //g_dx.deviceContext->Dispatch(1920, 1080, 1);

    //ID3D11ShaderResourceView* nullSRV[] = { NULL };
    //g_dx.deviceContext->CSSetShaderResources(0, 1, nullSRV);

    //// Unbind output from compute shader
    //ID3D11UnorderedAccessView* nullUAV[] = { NULL };
    //g_dx.deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    //// Disable Compute Shader
    //g_dx.deviceContext->CSSetShader(nullptr, nullptr, 0);

// 
}

VOID MyDirectX::InitDirectX(HWND hWnd)
{
// Outside Initialization
    InitDevice(hWnd);
    //CompileShaders();
// Input-Assembler Initialization
    CreateVertexBuffer();
}

VOID MyDirectX::FreeDirectX()
{
    g_dx.vertexLayout->Release();
    g_dx.swapChain->Release();
    g_dx.shaderResourceView->Release();
    g_dx.deviceContext->Release();
    g_dx.tex->Release();
    g_dx.computeShader->Release();
    g_dx.device->Release();
}

HRESULT MyDirectX::CreateVertexBuffer()
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(simpleVertex) * 3;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = g_dx.vertices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    HRESULT hr = g_dx.device->CreateBuffer(&bufferDesc, &InitData, &g_dx.vertexBuffer);
    return hr;
}

HRESULT MyDirectX::InitDevice(HWND hWnd)
{
// Outside of the Pipeline
    // Get the rectangle that contians the window, use it's 
    RECT rc;
    GetClientRect(hWnd, &rc);
    g_dx.width = rc.right - rc.left;
    g_dx.height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    // Create a description object which will be passed into a create object function that will set pointers to other newly initialized objects.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = g_dx.width;
    sd.BufferDesc.Height = g_dx.height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    HRESULT hr = S_OK;

    // Attempts to create a device and swap chain on the highest level of hardware acceleration.
    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        NULL, 0, D3D11_SDK_VERSION, &sd, &g_dx.swapChain,
        &g_dx.device, NULL, &g_dx.deviceContext);
    if (FAILED(hr))
    {
        // If failed, try without antialiasing
        sd.SampleDesc.Count = 1; // Check this line if buggy!!!
        hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
            NULL, 0, D3D11_SDK_VERSION, &sd, &g_dx.swapChain,
            &g_dx.device, NULL, &g_dx.deviceContext);
        if (FAILED(hr))
        {
            // If failed, try to create a reference device
            hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, createDeviceFlags,
                NULL, 0, D3D11_SDK_VERSION, &sd, &g_dx.swapChain,
                &g_dx.device, NULL, &g_dx.deviceContext);
            if (SUCCEEDED(hr))
                MessageBox(hWnd, L"No DX11 hardware acceleration found.\nSwitching to REFERENCE driver (very slow).",
                    L"Warning", MB_OK | MB_ICONWARNING);
            else
                return hr;
        }
    }
}

HRESULT MyDirectX::CompileShaders()
{
// Compile the Vertex Shader

// Compile the Pixel Shader

// Compile the ComputeShader
    ID3DBlob* csBlob = nullptr;
    HRESULT hr = CompileComputeShader(L"ComputeShader.hlsl", "main", g_dx.device, &csBlob);
    if (FAILED(hr))
    {
        g_dx.device->Release();
        return -1;
    }

    // Create The ComputeShader
    hr = g_dx.device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &g_dx.computeShader);

    csBlob->Release();

    if (FAILED(hr))
    {
        g_dx.device->Release();
    }

    // Create Output Buffers for the ComputeShader to write to.
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = g_dx.width;
    textureDesc.Height = g_dx.height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    hr = g_dx.device->CreateTexture2D(&textureDesc, 0, &g_dx.tex);
    g_dx.device->CreateShaderResourceView(g_dx.tex, NULL, &g_dx.shaderResourceView);
}

HRESULT MyDirectX::CompileComputeShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint,
    _In_ ID3D11Device* device, _Outptr_ ID3DBlob** blob)
{
    if (!srcFile || !entryPoint || !device || !blob)
        return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    // We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
    LPCSTR profile = "cs_5_0";

    const D3D_SHADER_MACRO defines[] =
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint, "cs_5_0",
        flags, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return hr;
    }

    *blob = shaderBlob;

    return hr;
}