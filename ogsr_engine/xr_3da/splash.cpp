#include "stdafx.h"

#include "splash.h"
#include "resource.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

static IStream* CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
{
    const HRSRC hrsrc = FindResource(nullptr, lpName, lpType);
    if (!hrsrc)
        return nullptr;

    const HGLOBAL hglbImage = LoadResource(nullptr, hrsrc);
    if (!hglbImage)
        return nullptr;

    const LPVOID pvSourceResourceData = LockResource(hglbImage);
    if (!pvSourceResourceData)
        return nullptr;

    const DWORD dwResourceSize = SizeofResource(nullptr, hrsrc);

    const HGLOBAL hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
    if (!hgblResourceData)
        return nullptr;

    const LPVOID pvResourceData = GlobalLock(hgblResourceData);
    if (!pvResourceData)
    {
        GlobalFree(hgblResourceData);
        return nullptr;
    }

    CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
    GlobalUnlock(hgblResourceData);

    IStream* ipStream = nullptr;
    if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
        return ipStream;

    GlobalFree(hgblResourceData);
    return nullptr;
}

const char* c_szSplashClass = "SplashWindow";

static HWND logoWindow = nullptr;
static HWND logoOwnerWindow = nullptr;

void ShowSplash(HINSTANCE hInstance)
{
    ULONG_PTR gdiplusToken = 0;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Gdiplus::Ok)
        return;

    CHAR path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);

    std::string splash_path{path};
    const auto last_slash = splash_path.find_last_of('\\');
    if (last_slash != std::string::npos)
        splash_path.erase(last_slash);
    splash_path += "\\splash.png";

    std::wstring splash_path_w(splash_path.begin(), splash_path.end());

    Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(splash_path_w.c_str());

    // Если картинки нет на диске, пробуем загрузить из ресурсов
    if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok)
    {
        delete pBitmap;
        pBitmap = nullptr;

        if (IStream* pStream = CreateStreamOnResource(MAKEINTRESOURCE(IDB_PNG1), TEXT("PNG")))
        {
            pBitmap = new Gdiplus::Bitmap(pStream);
            pStream->Release();
        }
    }

    if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok)
    {
        delete pBitmap;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    const int splashWidth = static_cast<int>(pBitmap->GetWidth());
    const int splashHeight = static_cast<int>(pBitmap->GetHeight());

    if (splashWidth <= 0 || splashHeight <= 0)
    {
        delete pBitmap;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = c_szSplashClass;
    RegisterClass(&wc);

    const HWND hwndOwner = CreateWindow(c_szSplashClass, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    const HWND hWnd = CreateWindowEx(WS_EX_LAYERED, c_szSplashClass, nullptr, WS_POPUP, 0, 0, 0, 0, hwndOwner, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        delete pBitmap;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    const HDC hdcScreen = GetDC(nullptr);
    const HDC hDC = CreateCompatibleDC(hdcScreen);

    // Создаем 32-bit DIB section для UpdateLayeredWindow
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = splashWidth;
    bmi.bmiHeader.biHeight = -splashHeight; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBmp = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    const HBITMAP hbmpOld = static_cast<HBITMAP>(SelectObject(hDC, hBmp));

    // Блокируем биты в GDI+ с автоматической пре-мультипликацией альфа-канала (PARGB)
    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, splashWidth, splashHeight);
    if (pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bmpData) == Gdiplus::Ok)
    {
        const int rowSize = splashWidth * 4;
        for (int y = 0; y < splashHeight; ++y)
        {
            memcpy(static_cast<BYTE*>(pvBits) + y * rowSize, static_cast<BYTE*>(bmpData.Scan0) + y * bmpData.Stride, rowSize);
        }
        pBitmap->UnlockBits(&bmpData);
    }

    delete pBitmap;
    pBitmap = nullptr;
    Gdiplus::GdiplusShutdown(gdiplusToken);

    // Alpha blending
    BLENDFUNCTION blend = {0};
    blend.BlendOp = AC_SRC_OVER;
    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.SourceConstantAlpha = 255;

    SIZE sizeWnd = {splashWidth, splashHeight};
    POINT ptPos = {0, 0};
    POINT ptSrc = {0, 0};

    if (const HWND hDT = GetDesktopWindow())
    {
        RECT rcDT;
        GetWindowRect(hDT, &rcDT);
        ptPos.x = (rcDT.right - splashWidth) / 2;
        ptPos.y = (rcDT.bottom - splashHeight) / 2;
    }

    UpdateLayeredWindow(hWnd, hdcScreen, &ptPos, &sizeWnd, hDC, &ptSrc, 0, &blend, LWA_ALPHA);

    const HWND logoInsertPos = IsDebuggerPresent() ? HWND_NOTOPMOST : HWND_TOPMOST;
    SetWindowPos(hWnd, logoInsertPos, 0, 0, splashWidth, splashHeight, SWP_NOMOVE | SWP_SHOWWINDOW);

    // Очистка GDI объектов
    SelectObject(hDC, hbmpOld);
    DeleteObject(hBmp);
    DeleteDC(hDC);
    ReleaseDC(nullptr, hdcScreen);

    logoOwnerWindow = hwndOwner;
    logoWindow = hWnd;
}

void HideSplash()
{
    if (logoWindow)
    {
        DestroyWindow(logoWindow);
        logoWindow = nullptr;
    }
    if (logoOwnerWindow)
    {
        DestroyWindow(logoOwnerWindow);
        logoOwnerWindow = nullptr;
    }
}
