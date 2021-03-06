#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace DDraw
{
	namespace ActivateAppHandler
	{
		void installHooks();
		void setCooperativeLevel(HWND hwnd, DWORD flags);
	}
}
