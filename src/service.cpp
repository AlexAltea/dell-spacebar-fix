/**
 * (c) 2017 Alexandro Sanchez Bach
 * Released under MIT license. Read LICENSE for more details.
 */

#include <cctype>
#include <cstdio>

#include <Windows.h>

#ifdef _DEBUG
#define LOGD(msg, ...) wprintf(msg "\n", __VA_ARGS__)
#else
#define LOGD
#endif

 // Configuration
#define SERVICE_NAME L"DellSpacebarFix"

// Service state
SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;

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

    // Process following subset of events
    if (wParam == WM_KEYDOWN || 
        wParam == WM_KEYUP)
    {
        WPARAM thisEvent = wParam;
        DWORD thisKey = pKeyBoard->vkCode;
        DWORD thisTime = pKeyBoard->time;

        if (IsAlphaNumKey(thisKey))
            LOGD(L"[%d] Key %02X %s", thisTime, thisKey,
                (wParam == WM_KEYDOWN ? L"pressed" : L"released"));

        // Filter out #2 from (#1, #2)
        if (lastKey == VK_SPACE && lastEvent == WM_KEYDOWN &&
            thisKey == VK_SPACE && thisEvent == WM_KEYUP &&
            thisTime == lastTime) {
            return 1;
        }
        // Filter out #4 from (#3, #4)
        if (lastKey != VK_SPACE && lastEvent == WM_KEYDOWN &&
            thisKey == VK_SPACE && thisEvent == WM_KEYDOWN &&
            thisTime == lastTime) {
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

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    HINSTANCE instance = GetModuleHandle(NULL);
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);

    MSG msg;
    while (!GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hook);
    return ERROR_SUCCESS;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        // Preparations to stop the service
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            LOGD(L"%s: My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error", SERVICE_NAME);
        }

        // Signal the service worker
        PostQuitMessage(0);
        SetEvent(g_ServiceStopEvent);
        break;

    default:
        break;
    }
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    // Register the service control handler
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_StatusHandle == NULL) {
        LOGD(L"%s: My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error", SERVICE_NAME);
        goto exit;
    }

    // Tell the service controller we are starting
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        LOGD(L"%s: My Sample Service: ServiceMain: SetServiceStatus returned error", SERVICE_NAME);
        goto exit;
    }

    // Create a service stop event to wait on later
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) {
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            LOGD(L"%s: My Sample Service: ServiceMain: SetServiceStatus returned error", SERVICE_NAME);
        }
        goto exit;
    }

    // Inform the service controller
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        LOGD(L"%s: ServiceMain: SetServiceStatus returned error", SERVICE_NAME);
        goto exit;
    }

    // Worker thread
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    // Cleanup
    CloseHandle(g_ServiceStopEvent);

    // Inform the service controller
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        LOGD(L"%s: ServiceMain: SetServiceStatus returned error", SERVICE_NAME);
        goto exit;
    }

exit:
    return;
}

int main(int argc, char *argv[])
{
	// Launched as service
	if (1){//argc >= 2 && !strcmp(argv[1], "service")) {
		SERVICE_TABLE_ENTRY ServiceTable[] = {
			{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
			{ NULL, NULL }
		};

		if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
			return GetLastError();
		}
	}
	UnhookWindowsHookEx(hook);
    return 0;
}
