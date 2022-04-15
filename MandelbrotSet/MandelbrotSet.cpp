// MandelbrotSet.cpp : Defines the entry point for the application.
//
// Things I've learned:
// Windows likes to do a lot of object initialization by creating a description object, setting the values, and passing it into a function call
// this is nice and usefull. Remember this for the future and maybe even use it in your own code.

#include "framework.h"
#include "MandelbrotSet.h"
#include <objidl.h>

#define MAX_LOADSTRING 100

int redVal = 255;

IDXGISwapChain* g_swapChain;
ID3D11Device* g_device;
ID3D11DeviceContext* g_deviceContext;
ID3D11ComputeShader* g_computeShader = nullptr;
ID3D11Texture2D* g_tex;
ID3D11ShaderResourceView* g_shaderResourceView;

DXGI_SWAP_CHAIN_DESC sd;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text (Used to create the window in InitInstance())
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void OnPaint(HDC hdc)
{
// Input-Assembler Stage

// Compute Shatder Stage, writes to a texture that can hopefully be accessed later when we need to map it to our vertices.
    g_deviceContext->CSSetShader(g_computeShader, nullptr, 0);
    g_deviceContext->CSSetShaderResources(0, 1, &g_shaderResourceView);
    g_deviceContext->Dispatch(1920, 1080, 1);

    ID3D11ShaderResourceView* nullSRV[] = { NULL };
    g_deviceContext->CSSetShaderResources(0, 1, nullSRV);

    // Unbind output from compute shader
    ID3D11UnorderedAccessView* nullUAV[] = { NULL };
    g_deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    // Disable Compute Shader
    g_deviceContext->CSSetShader(nullptr, nullptr, 0);
}

HRESULT CompileComputeShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint,
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

HRESULT InitDevice(HWND hWnd)
{
    // Get the size of the window that we are drawing to.
    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 144;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    HRESULT hr = S_OK;

    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        NULL, 0, D3D11_SDK_VERSION, &sd, &g_swapChain,
        &g_device, NULL, &g_deviceContext);
    if (FAILED(hr))
    {
        // If failed, try without antialiasing
        sd.SampleDesc.Count = 1; // Check this line if buggy!!!
        hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
            NULL, 0, D3D11_SDK_VERSION, &sd, &g_swapChain,
            &g_device, NULL, &g_deviceContext);
        if (FAILED(hr))
        {
            // If failed, try to create a reference device
            hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, createDeviceFlags,
                NULL, 0, D3D11_SDK_VERSION, &sd, &g_swapChain,
                &g_device, NULL, &g_deviceContext);
            if (SUCCEEDED(hr))
                MessageBox(hWnd, L"No DX11 hardware acceleration found.\nSwitching to REFERENCE driver (very slow).",
                    L"Warning", MB_OK | MB_ICONWARNING);
            else
                return hr;
        }
    }

    // Compile the ComputeShader
    ID3DBlob* csBlob = nullptr;
    hr = CompileComputeShader(L"ComputeShader.hlsl", "main", g_device, &csBlob);
    if (FAILED(hr))
    {
        g_device->Release();
        return -1;
    }

    // Create The ComputeShader
    hr = g_device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &g_computeShader);

    csBlob->Release();

    if (FAILED(hr))
    {
        g_device->Release();
    }

    // Create Output Buffers for the ComputeShader to writer.
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    g_tex = 0;
    hr = g_device->CreateTexture2D(&textureDesc, 0, &g_tex);
    g_device->CreateShaderResourceView(g_tex, NULL, &g_shaderResourceView);
}

// This is the entry to the program. APIENTRY is short for WINAPI, and wWinMain is just WinMain that takes an input of a wchar rather than a char
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MANDELBROTSET, szWindowClass, MAX_LOADSTRING);

    // We populate this one structure that does some stuff
    // "The structure contains information about the window: the application icon, the background color of the window,
    // the name to display in the title bar, among other things. Importantly, it contains a function pointer to your window procedure."
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MANDELBROTSET));

    MSG msg;

    // Main message loop:
    // Constantly checks for messages, then translates them.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Cleanup
    g_computeShader->Release();
    g_device->Release();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    // Runs at the start of WinMain to populate this one struct
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MANDELBROTSET));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MANDELBROTSET);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    // Finally go and actually register the class by passing the address into this one register class function.
    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   // Runs to initiailize the window in the middle of wWinMain?
   hInst = hInstance; // Store instance handle in our global variable

   // Creates the actual window itself.
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      // Stop trying to show the window if window creation didn't work.
      return FALSE;
   }

   // Set up DirectX to display to the new window
   InitDevice(hWnd);

   // Show the window that we just created.
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
// A second entry point into our program that runs repeatedly, handling the messages that are translated in the loop in wWinMain.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            OnPaint(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
