

#include <windows.h>
#include <iostream>
//#include "DrawOverlay.h"

HWND hwndOverlay;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// Set the background to transparent
		SetBkMode(hdc, TRANSPARENT);

		// Draw a rectangle
		RECT rect = { 50, 50, 200, 200 };
		FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
		FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

		// Draw a circle
		Ellipse(hdc, 250, 50, 400, 200);

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


HWND CreateOverlayWindow(HINSTANCE hInstance) {
	// Create a window class
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "OverlayWindowClass";
	RegisterClass(&wc);

	// Create the overlay window
	HWND hwnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
		"OverlayWindowClass",
		"Overlay",
		WS_POPUP | WS_VISIBLE,
		0, 200, 500, 500,										// Position and size of the window
		NULL, NULL, hInstance, NULL
	);

	// Set the window to be transparent
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 128, LWA_ALPHA); // Adjust transparency level

	return hwnd;
}

void CloseOverlay(HWND hwnd) {
	PostMessage(hwnd, WM_CLOSE, 0, 0);
}

void ShowOverlay() {
	// Get the instance handle
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Create and show the overlay window
	//HWND hwnd = CreateOverlayWindow(hInstance);
	hwndOverlay = CreateOverlayWindow(hInstance);

	// Ensure the window is shown in normal mode, not minimized
	ShowWindow(hwndOverlay, SW_SHOWNORMAL);
	UpdateWindow(hwndOverlay);

	//Sleep(5000); 
	//CloseOverlay(hwnd);

	// Main message loop
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//return 0;
}
void DeleteOverlay() {
	PostMessage(hwndOverlay, WM_CLOSE, 0, 0);
}




