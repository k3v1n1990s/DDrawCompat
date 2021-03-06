#include "DDraw/Surfaces/PrimarySurface.h"
#include "Gdi/Caret.h"
#include "Gdi/Dc.h"
#include "Gdi/DcCache.h"
#include "Gdi/DcFunctions.h"
#include "Gdi/Gdi.h"
#include "Gdi/PaintHandlers.h"
#include "Gdi/Palette.h"
#include "Gdi/ScrollFunctions.h"
#include "Gdi/Window.h"
#include "Gdi/WinProc.h"

namespace
{
	HDC g_screenDc = nullptr;

	BOOL CALLBACK redrawWindowCallback(HWND hwnd, LPARAM lParam)
	{
		DWORD windowPid = 0;
		GetWindowThreadProcessId(hwnd, &windowPid);
		if (GetCurrentProcessId() == windowPid)
		{
			Gdi::redrawWindow(hwnd, reinterpret_cast<HRGN>(lParam));
		}
		return TRUE;
	}
}

namespace Gdi
{
	void dllThreadDetach()
	{
		WinProc::dllThreadDetach();
		Dc::dllThreadDetach();
		DcCache::dllThreadDetach();
	}

	HDC getScreenDc()
	{
		return g_screenDc;
	}

	HRGN getVisibleWindowRgn(HWND hwnd)
	{
		return DcFunctions::getVisibleWindowRgn(hwnd);
	}

	void installHooks()
	{
		// Workaround for VirtualizeDesktopPainting shim, which doesn't seem to handle BitBlt
		// from screen DC to screen DC correctly
		auto getDc = reinterpret_cast<decltype(GetDC)*>(Compat::getProcAddress(GetModuleHandle("user32"), "GetDC"));
		if (!getDc)
		{
			getDc = &GetDC;
		}
		g_screenDc = getDc(nullptr);

		DcFunctions::installHooks();
		PaintHandlers::installHooks();
		Palette::installHooks();
		ScrollFunctions::installHooks();
		Window::installHooks();
		WinProc::installHooks();
		Caret::installHooks();
	}

	bool isDisplayDc(HDC dc)
	{
		return dc && OBJ_DC == GetObjectType(dc) && DT_RASDISPLAY == CALL_ORIG_FUNC(GetDeviceCaps)(dc, TECHNOLOGY) &&
			!(GetWindowLongPtr(CALL_ORIG_FUNC(WindowFromDC)(dc), GWL_EXSTYLE) & WS_EX_LAYERED);
	}

	void redraw(HRGN rgn)
	{
		EnumWindows(&redrawWindowCallback, reinterpret_cast<LPARAM>(rgn));
	}

	void redrawWindow(HWND hwnd, HRGN rgn)
	{
		if (!IsWindowVisible(hwnd) || IsIconic(hwnd) || Window::isPresentationWindow(hwnd))
		{
			return;
		}

		POINT origin = {};
		if (rgn)
		{
			ClientToScreen(hwnd, &origin);
			OffsetRgn(rgn, -origin.x, -origin.y);
		}

		RedrawWindow(hwnd, nullptr, rgn, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

		if (rgn)
		{
			OffsetRgn(rgn, origin.x, origin.y);
		}
	}

	void unhookWndProc(LPCSTR className, WNDPROC oldWndProc)
	{
		HWND hwnd = CreateWindow(className, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, 0);
		SetClassLongPtr(hwnd, GCLP_WNDPROC, reinterpret_cast<LONG>(oldWndProc));
		DestroyWindow(hwnd);
	}

	void uninstallHooks()
	{
		Caret::uninstallHooks();
		WinProc::uninstallHooks();
		Window::uninstallHooks();
		Dc::dllProcessDetach();
		DcCache::dllProcessDetach();
		CALL_ORIG_FUNC(ReleaseDC)(nullptr, g_screenDc);
	}

	void watchWindowPosChanges(WindowPosChangeNotifyFunc notifyFunc)
	{
		WinProc::watchWindowPosChanges(notifyFunc);
	}
}
