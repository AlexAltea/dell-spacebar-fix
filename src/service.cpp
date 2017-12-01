/**
 * (c) 2017 Alexandro Sanchez Bach
 * Released under MIT license. Read LICENSE for more details.
 */

#include <cctype>
#include <cstdio>

#include <Windows.h>

#ifdef _DEBUG
#define LOGD(msg, ...) printf(msg "\n", __VA_ARGS__)
#else
#define LOGD
#endif

bool IsAlphaNumKey(DWORD vkCode)
{
    return
        (vkCode >= 0x30 && vkCode <= 0x39) ||  // [0-9]
        (vkCode >= 0x41 && vkCode <= 0x5A);    // [A-Z]
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;

    // Global hook state
    static WPARAM lastEvent = 0;
    static DWORD lastKey = 0;
    static DWORD lastTime = 0;
    static bool dropNext = false;

    // Process following subset of events
    if (wParam == WM_KEYDOWN || 
        wParam == WM_KEYUP)
    {
        WPARAM thisEvent = wParam;
        DWORD thisKey = pKeyBoard->vkCode;
        DWORD thisTime = pKeyBoard->time;

        if (IsAlphaNumKey(thisKey))
            LOGD("[%d] Key %02X %s", thisTime, thisKey,
                (wParam == WM_KEYDOWN ? "pressed" : "released"));

        // Filter out #2 from (#1, #2)
        if (lastKey == VK_SPACE && lastEvent == WM_KEYDOWN &&
            thisKey == VK_SPACE && thisEvent == WM_KEYUP &&
            thisTime == lastTime) {
            dropNext = true;
            return 1;
        }
        // Filter out #4 from (#3, #4) for doubled spaces
        if (lastKey != VK_SPACE && lastEvent == WM_KEYUP &&
            thisKey == VK_SPACE && thisEvent == WM_KEYDOWN &&
            thisTime == lastTime && dropNext) {
            dropNext = false;
            return 1;
        }
        // Filter out #4 from (#3, #4) for inserted spaces
        if (lastKey != VK_SPACE && lastEvent == WM_KEYDOWN &&
            thisKey == VK_SPACE && thisEvent == WM_KEYDOWN &&
            thisTime == lastTime && dropNext) {
            dropNext = false;
            return 1;
        }

        // Update state
        lastEvent = thisEvent;
        lastKey = thisKey;
        lastTime = thisTime;
    }

    // Forward the event
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	HINSTANCE instance = GetModuleHandle(NULL);
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);

	MSG msg;
	while (!GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(hook);
    return 0;
}
