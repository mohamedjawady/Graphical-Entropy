#include "framework.h"
#include "GraphicalEntropy.h"
#include "Utility.h"
#include <ShellAPI.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>

// Function prototypes
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void DrawEntropyAndPath(HDC hdc);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnDropFiles(HDROP hDrop, HWND hWnd);
void CalculateWindowSize(HWND hWnd, int& windowWidth, int& windowHeight);

// Global variables
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
double currentEntropy = 0.0;
std::wstring currentFilePath;
size_t currentFileSize = 0;
std::wstring currentFileType;
std::wstring currentFileFormat;
bool fileDropped = false;

// Entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRAPHICALENTROPY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPHICALENTROPY));

    MSG msg;

    // Main message loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

// Register window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAPHICALENTROPY));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GRAPHICALENTROPY);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Initialize the application instance
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    // Calculate the required window size
    int windowWidth, windowHeight;
    CalculateWindowSize(nullptr, windowWidth, windowHeight);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU,
        CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        DragAcceptFiles(hWnd, TRUE);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
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

        DrawEntropyAndPath(hdc);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_DROPFILES:
        OnDropFiles(reinterpret_cast<HDROP>(wParam), hWnd);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Handle dropped files
void OnDropFiles(HDROP hDrop, HWND hWnd)
{
    UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);

    if (fileCount == 1)
    {
        WCHAR filePath[MAX_PATH];
        DragQueryFile(hDrop, 0, filePath, MAX_PATH);

        std::vector<unsigned char> fileData;
        std::ifstream fileStream(filePath, std::ios::binary);

        if (fileStream.is_open())
        {
            fileData.assign(std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>());
            fileStream.close();
        }
        else
        {
            MessageBox(nullptr, L"Error reading file!", L"Error", MB_OK | MB_ICONERROR);
            return;
        }

        currentEntropy = CalculateEntropy(fileData);
        currentFilePath = filePath;
        currentFileSize = fileData.size();
        currentFileType = IsBinary(fileData) ? L"Binary" : L"Text";
        currentFileFormat = GetFileFormat(filePath);

        fileDropped = true;

        // Force window repaint to display updated information
        InvalidateRect(hWnd, NULL, TRUE);
    }
    else
    {
        MessageBox(nullptr, L"Please drop only one file at a time", L"File Drop", MB_OK | MB_ICONWARNING);
    }

    DragFinish(hDrop);
}

// About dialog procedure
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

// Draw entropy and file path information
void DrawEntropyAndPath(HDC hdc)
{
    int x = 50;
    int y = -150;
    int lineHeight = 80;
    int columnWidth = 300;

    RECT tableRect = { x, y, x + 3 * columnWidth, y + 5 * lineHeight };

    RECT clientRect;
    GetClientRect(WindowFromDC(hdc), &clientRect);

    int centerX = (clientRect.right - clientRect.left) / 2;
    int centerY = (clientRect.bottom - clientRect.top) / 2;

    RECT textRect = clientRect;

    if (!fileDropped)
    {
        std::wstring initialMessage = L"Drag and drop a file here";

        // Calculate the text size
        SIZE textSize;
        GetTextExtentPoint32(hdc, initialMessage.c_str(), initialMessage.length(), &textSize);

        // Center the text horizontally and vertically
        int textX = centerX - textSize.cx / 2;
        int textY = centerY - textSize.cy / 2;

        // Draw the centered text
        DrawText(hdc, initialMessage.c_str(), initialMessage.length(), &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else
    {
        // Draw the table headers
        DrawText(hdc, L"Property", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tableRect.left += columnWidth;
        DrawText(hdc, L"Value", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Draw the table data
        tableRect.left = x;
        tableRect.top += lineHeight;
        DrawText(hdc, L"Entropy:", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tableRect.left += columnWidth;
        std::wstring entropyText = std::to_wstring(currentEntropy);
        DrawText(hdc, entropyText.c_str(), entropyText.length(), &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Repeat the above steps for other properties (File Path, File Size, etc.)

        tableRect.left = x;
        tableRect.top += lineHeight;
        DrawText(hdc, L"File Path:", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tableRect.left += columnWidth;
        DrawText(hdc, currentFilePath.c_str(), currentFilePath.length(), &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        tableRect.left = x;
        tableRect.top += lineHeight;
        DrawText(hdc, L"File Size:", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tableRect.left += columnWidth;
        std::wstring fileSizeText = std::to_wstring(currentFileSize) + L" bytes";
        DrawText(hdc, fileSizeText.c_str(), fileSizeText.length(), &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Repeat the above steps for other properties (File Type, File Format)

        // ... (additional properties)

        tableRect.left = x;
        tableRect.top += lineHeight;
        DrawText(hdc, L"File Type:", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tableRect.left += columnWidth;
        DrawText(hdc, currentFileType.c_str(), currentFileType.length(), &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        tableRect.left = x;
        tableRect.top += lineHeight;
        DrawText(hdc, L"File Format:", -1, &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tableRect.left += columnWidth;
        DrawText(hdc, currentFileFormat.c_str(), currentFileFormat.length(), &tableRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
}

void CalculateWindowSize(HWND hWnd, int& windowWidth, int& windowHeight)
{
    int x = 50;
    int y = -150;
    int lineHeight = 80;
    int columnWidth = 300;
    int numRows = 5;  // Number of rows in the table

    windowWidth = x + 4 * columnWidth;
    windowHeight = y + (numRows+1) * lineHeight;
}

