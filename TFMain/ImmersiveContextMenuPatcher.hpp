#pragma once
#include "pch.h"

namespace TranslucentFlyouts
{
	// Tweak the appearance of immersive context menu
	namespace ImmersiveContextMenuPatcher
	{
		HRESULT WINAPI DrawThemeBackground(
			HTHEME  hTheme,
			HDC     hdc,
			int     iPartId,
			int     iStateId,
			LPCRECT pRect,
			LPCRECT pClipRect
		);

		void Startup();
		void Shutdown();
	}
}