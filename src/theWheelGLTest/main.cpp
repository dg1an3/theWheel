/// theWheelGLTest/main.cpp
/// Minimal Win32 test harness for theWheelGL renderer via ANGLE.
/// Renders plaques at various activation levels for visual verification.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "GLRenderer.h"
#include "GLNodeView.h"
#include "GLNodeViewSkin.h"
#include "GLMath.h"

using namespace theWheelGL;

static GLRenderer g_renderer;
static GLNodeViewSkin g_skin;
static bool g_initialized = false;

static void RenderScene()
{
    if (!g_initialized) return;

    int w = g_renderer.GetWidth();
    int h = g_renderer.GetHeight();

    g_renderer.BeginFrame(232.0f/255.0f, 232.0f/255.0f, 232.0f/255.0f);

    // View/projection matching SpacePanel and D3D9 SpaceView
    Vec3f eye(0.0f, 0.0f, 5.0f);
    Vec3f at(0.0f, 0.0f, 0.0f);
    Vec3f up(0.0f, 1.0f, 0.0f);
    g_renderer.SetViewMatrix(Mat4f::LookAtLH(eye, at, up));
    g_renderer.SetProjectionMatrix(Mat4f::OrthoLH((float)w, (float)h, -40.0f, 40.0f));

    // Directional light matching D3D9 SpaceView
    g_renderer.SetLight(Vec3f(-0.2f, -0.3f, -0.5f), Vec3f(1.0f, 1.0f, 1.0f));
    g_renderer.SetAmbientLight(Vec3f(0.25f, 0.25f, 0.25f));

    // Render plaques at various activation levels, spaced horizontally
    const float activations[] = { 0.05f, 0.1f, 0.2f, 0.5f, 1.0f };
    const int count = 5;
    float spacing = (float)w / (float)(count + 1);

    for (int i = 0; i < count; i++) {
        float x = spacing * (i + 1) - (float)w / 2.0f;
        float y = 0.0f;
        GLNodeView::Draw(g_renderer, g_skin, x, y, 0.0f, activations[i]);
    }

    g_renderer.EndFrame();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        return 0;

    case WM_SIZE: {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        if (w > 0 && h > 0 && g_initialized) {
            g_renderer.Resize(w, h);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        RenderScene();
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        g_renderer.Shutdown();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"theWheelGLTest";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"theWheelGL Test — ANGLE Renderer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    ShowWindow(hwnd, nCmdShow);

    // Initialize ANGLE renderer
    RECT rc;
    GetClientRect(hwnd, &rc);
    g_initialized = g_renderer.InitFromWindow((void*)hwnd,
        rc.right - rc.left, rc.bottom - rc.top);

    if (!g_initialized) {
        MessageBoxW(hwnd, L"Failed to initialize ANGLE/EGL renderer.",
                    L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Force initial paint
    InvalidateRect(hwnd, nullptr, FALSE);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
