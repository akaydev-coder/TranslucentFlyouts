#pragma once
#include "pch.h"

class ThemeHelper
{
public:
	typedef HRESULT(WINAPI * pfnGetThemeClass)(HTHEME hTheme, LPCWSTR pszClassName, int cchClassName);
	typedef BOOL(WINAPI*pfnIsTopLevelWindow)(HWND);

	static inline bool IsAllowTransparent()
	{
		HKEY hKey = nullptr;
		LONG lResult;
		BYTE bResult = true;
		DWORD dwSize = sizeof(DWORD);
		lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, L"EnableTransparency", NULL, nullptr, &bResult, &dwSize);
			RegCloseKey(hKey);
		}
		return bResult;
	}

	static inline bool IsAppLightTheme()
	{
		HKEY hKey = nullptr;
		LONG lResult;
		BYTE bResult = true;
		DWORD dwSize = sizeof(DWORD);
		lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, L"AppsUseLightTheme", NULL, nullptr, &bResult, &dwSize);
			RegCloseKey(hKey);
		}
		return bResult;
	}

	static inline bool IsSystemLightTheme()
	{
		HKEY hKey = nullptr;
		LONG lResult;
		BYTE bResult = true;
		DWORD dwSize = sizeof(DWORD);
		lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, L"SystemUsesLightTheme", NULL, nullptr, &bResult, &dwSize);
			RegCloseKey(hKey);
		}
		return bResult;
	}

	static inline HRESULT WINAPI GetThemeClass(const HTHEME& hTheme, LPCWSTR pszClassName, const int cchClassName)
	{
		static const pfnGetThemeClass GetThemeClass = (pfnGetThemeClass)GetProcAddress(GetModuleHandle(TEXT("Uxtheme")), MAKEINTRESOURCEA(74));
		if (GetThemeClass)
		{
			return GetThemeClass(hTheme, pszClassName, cchClassName);
		}
		else
		{
			return E_POINTER;
		}
	}

	static inline bool VerifyThemeData(const HTHEME& hTheme, LPCWSTR pszThemeClassName)
	{
		WCHAR pszClassName[MAX_PATH + 1];
		GetThemeClass(hTheme, pszClassName, MAX_PATH);
		return !_wcsicmp(pszClassName, pszThemeClassName);
	}

	static inline BOOL IsTopLevelWindow(const HWND& hWnd)
	{
		static const pfnIsTopLevelWindow IsTopLevelWindow = (pfnIsTopLevelWindow)GetProcAddress(GetModuleHandle(L"User32"), "IsTopLevelWindow");
		if (IsTopLevelWindow)
		{
			return IsTopLevelWindow(hWnd);
		}
		return FALSE;
	}

	static inline bool IsValidFlyout(const HWND& hWnd)
	{
		WCHAR szClass[MAX_PATH + 1];
		GetClassName(hWnd, szClass, MAX_PATH);
		return (
		           IsTopLevelWindow(hWnd) and
		           (
		               !wcscmp(szClass, L"#32768") or
		               !wcscmp(szClass, L"ViewControlClass") or
		               (!wcscmp(szClass, L"DropDown") and !FindWindowEx(hWnd, nullptr, L"ListviewPopup", L"Suggest"))
		           )
		       );
	}

	static inline void Clear(HDC hdc, LPCRECT lpRect)
	{
		PatBlt(
		    hdc,
		    lpRect->left,
		    lpRect->top,
		    lpRect->right - lpRect->left,
		    lpRect->bottom - lpRect->top,
		    BLACKNESS
		);
	}

	static inline HBITMAP CreateDIB(HDC hdc, LONG nWidth, LONG nHeight, PVOID* pvBits)
	{
		BITMAPINFO bitmapInfo = {};
		bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biWidth = nWidth;
		bitmapInfo.bmiHeader.biHeight = -nHeight;

		return CreateDIBSection(hdc, &bitmapInfo, DIB_RGB_COLORS, pvBits, NULL, 0);
	}

	static inline COLORREF GetBrushColor(const HBRUSH& hBrush)
	{
		LOGBRUSH lbr;
		GetObject(hBrush, sizeof(lbr), &lbr);
		if (lbr.lbStyle != BS_SOLID)
		{
			return CLR_NONE;
		}
		return lbr.lbColor;
	}

	static inline void SetPixel(PBYTE pvBits, BYTE b, BYTE g, BYTE r, BYTE a)
	{
		// AlphaԤ��
		pvBits[0] = (b * (a + 1)) >> 8;
		pvBits[1] = (g * (a + 1)) >> 8;
		pvBits[2] = (r * (a + 1)) >> 8;
		pvBits[3] = a;
	}

	static inline void HighlightBox(HDC hdc, LPCRECT lpRect, COLORREF dwColor)
	{
		HPEN hPen = CreatePen(PS_SOLID, 3, dwColor);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
	}

	static inline bool IsUsing32BPP(HDC hdc)
	{
		HBITMAP hBitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
		if (hBitmap)
		{
			DIBSECTION ds = {};
			if (GetObject(hBitmap, sizeof(ds), &ds) == sizeof(ds) and ds.dsBm.bmBitsPixel == 32 and ds.dsBmih.biCompression == BI_RGB)
			{
				return true;
			}
		}
		return false;
	}

	static inline bool IsPixelContainAlpha(const DIBSECTION& ds)
	{
		int i = 0;
		BYTE* pbPixel = nullptr;
		bool bHasAlpha = false;
		
		if (ds.dsBm.bmBitsPixel != 32 or ds.dsBmih.biCompression != BI_RGB)
		{
			return false;
		}

		for (i = 0, pbPixel = (BYTE*)ds.dsBm.bmBits; i < ds.dsBmih.biWidth * ds.dsBmih.biHeight; i++, pbPixel += 4)
		{
			if ((bHasAlpha = (pbPixel[3] != 0xff)))
			{
				break;
			}
		}

		if (!bHasAlpha)
		{
			return false;
		}

		return true;
	}

	static bool IsBitmapSupportAlpha(HBITMAP hBitmap)
	{
		bool bResult = false;
		HDC hdc = CreateCompatibleDC(NULL);
		BITMAPINFO BitmapInfo = {sizeof(BitmapInfo.bmiHeader)};

		if (GetDIBits(hdc, hBitmap, 0, 0, NULL, &BitmapInfo, DIB_RGB_COLORS))
		{
			BYTE *pvBits = new BYTE[BitmapInfo.bmiHeader.biSizeImage];

			BitmapInfo.bmiHeader.biCompression = BI_RGB;
			BitmapInfo.bmiHeader.biBitCount = 32;

			if (pvBits and GetDIBits(hdc, hBitmap, 0, BitmapInfo.bmiHeader.biHeight, (LPVOID)pvBits, &BitmapInfo, DIB_RGB_COLORS))
			{
				for (UINT i = 0; i < BitmapInfo.bmiHeader.biSizeImage; i += 4)
				{
					if (pvBits[i + 3] == 0xff)
					{
						bResult = true;
						break;
					}
				}
			}
			delete[] pvBits;
		}

		DeleteDC(hdc);
		return bResult;
	}

	static void Convert24To32BPP(HBITMAP hBitmap)
	{
		HDC hdc = CreateCompatibleDC(NULL);
		BITMAPINFO BitmapInfo = {sizeof(BitmapInfo.bmiHeader)};

		if (GetDIBits(hdc, hBitmap, 0, 0, NULL, &BitmapInfo, DIB_RGB_COLORS))
		{
			BYTE *pvBits = new BYTE[BitmapInfo.bmiHeader.biSizeImage];

			BitmapInfo.bmiHeader.biCompression = BI_RGB;
			BitmapInfo.bmiHeader.biBitCount = 32;

			if (pvBits and GetDIBits(hdc, hBitmap, 0, BitmapInfo.bmiHeader.biHeight, (LPVOID)pvBits, &BitmapInfo, DIB_RGB_COLORS))
			{
				for (UINT i = 0; i < BitmapInfo.bmiHeader.biSizeImage; i += 4)
				{
					pvBits[i] = (pvBits[i] * 256) >> 8;
					pvBits[i + 1] = (pvBits[i + 1] * 256) >> 8;
					pvBits[i + 2] = (pvBits[i + 2] * 256) >> 8;
					pvBits[i + 3] = 255;
				}

				SetDIBits(hdc, hBitmap, 0, BitmapInfo.bmiHeader.biHeight, pvBits, &BitmapInfo, DIB_RGB_COLORS);
			}
			delete[] pvBits;
		}

		DeleteDC(hdc);
	}
};