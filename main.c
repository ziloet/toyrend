#include<windows.h>

#define Assert(Condition) if(Condition); else __debugbreak()

static LRESULT CALLBACK
window_proc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		default:
		{
			Result = DefWindowProcW(Window, Message, wParam, lParam);
		}
	}
	return Result;
}

static HWND
create_window(unsigned DesiredWidth, unsigned DesiredHeight)
{
	HINSTANCE Instance = GetModuleHandleW(0);
	WCHAR* WindowClassName = L"wndclass_name";
	WNDCLASSEXW WindowClass = {0};
	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = window_proc;
	WindowClass.lpszClassName = WindowClassName;
	RegisterClassExW(&WindowClass);
	DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT WindowRect = {0, 0, DesiredWidth, DesiredHeight};
	AdjustWindowRect(&WindowRect, WindowStyle, 0);
	unsigned WindowWidth = WindowRect.right - WindowRect.left;
	unsigned WindowHeight = WindowRect.bottom - WindowRect.top;
	HWND Result = CreateWindowExW(0, WindowClassName, L"toyrend", WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight, 0, 0, Instance, 0);
	Assert(IsWindow(Result));
	return Result;
}

static void
disable_dpi_scaling(void)
{
	HMODULE Library = LoadLibraryW(L"user32.dll");
	void* Function = GetProcAddress(Library, "SetProcessDpiAwarenessContext");
	if(Function)
	{
		typedef BOOL dpi_new_function(DPI_AWARENESS_CONTEXT);
		dpi_new_function* SetProcessDpiAwarenessCotext = Function;
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		return;
	}
	Function = GetProcAddress(Library, "SetProcessDpiAware");
	if(Function)
	{
		typedef BOOL dpi_old_function(void);
		dpi_old_function* SetProcessDpiAware = Function;
		SetProcessDpiAware();
	}
}

void WinMainCRTStartup()
{
	disable_dpi_scaling();
	HWND Window = create_window(1024, 768);

	for(;;)
	{
		MSG Message;
		GetMessageW(&Message, 0, 0, 0);
		do
		{
			if(Message.message == WM_QUIT)
			{
				ExitProcess(0);
			}
			DispatchMessageW(&Message);
		} while(PeekMessageW(&Message, 0, 0, 0, PM_REMOVE));
	}
}
