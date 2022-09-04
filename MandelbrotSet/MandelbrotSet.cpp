// MandelbrotSet.cpp : Defines the entry point for the application.
//
// Things I've learned:
// Windows likes to do a lot of object initialization by creating a description object, setting the values, and passing it into a function call
// this is nice and usefull. Remember this for the future and maybe even use it in your own code.

#include "framework.h"
#include "MandelbrotSet.h"
#include "MyDirectX.h"
#include <objidl.h>

#define MAX_LOADSTRING 100

MyDirectX m_dx;

int redVal = 255;
bool mandelbrotDrawReady = true;

RECT selectedRect = { 0 };

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
    m_dx.Draw();
}

void UpdateMandelbrotBuffer(HWND hWnd)
{
    m_dx.ResizeDevices(hWnd);
}

void HandleMouseLeftDown(LPARAM input)
{
    int xPos = GET_X_LPARAM(input);
    int yPos = GET_Y_LPARAM(input);

    selectedRect.top = yPos;
    selectedRect.left = xPos;
}

void HandleMouseLeftHold(LPARAM input)
{
    int xPos = GET_X_LPARAM(input);
    int yPos = GET_Y_LPARAM(input);

    selectedRect.bottom = yPos;
    selectedRect.right = xPos;
}

void HandleMouseLeftUp(LPARAM input)
{
    int xPos = GET_X_LPARAM(input);
    int yPos = GET_Y_LPARAM(input);

    selectedRect.bottom = yPos;
    selectedRect.right = xPos;
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
   SetMenu(hWnd, NULL);

   if (!hWnd)
   {
      // Stop trying to show the window if window creation didn't work.
      return FALSE;
   }

   // Run our code to initialize DirectX.
   m_dx.InitDirectX(hWnd);

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
        if (mandelbrotDrawReady)
        {
            OnPaint(hdc);
            mandelbrotDrawReady = false;
        }
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        HandleMouseLeftDown(lParam);
    }
    break;
    case WM_LBUTTONUP:
    {
        HandleMouseLeftUp(lParam);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_EXITSIZEMOVE:
    {
        mandelbrotDrawReady = true;
        UpdateMandelbrotBuffer(hWnd);
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }
    break;
    case WM_SYSCHAR:
    {
        mandelbrotDrawReady = true;
        UpdateMandelbrotBuffer(hWnd);
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }
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
