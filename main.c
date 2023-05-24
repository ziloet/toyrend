#include<windows.h>
#include<stdint.h>

#include"toyrend.h"

#include"toyrend_renderer.c"

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

		case WM_SIZE:
		{
			gdi_renderer* Renderer = (gdi_renderer*)GetWindowLongPtrW(Window, GWLP_USERDATA);
			break_if(!Renderer);
			renderer_resize(Renderer);
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
	HDC WindowDC = GetDC(Window);
	gdi_renderer* Renderer = renderer_create(Window);
	SetWindowLongPtrW(Window, GWLP_USERDATA, (LONG_PTR)Renderer);
	uint32_t X0 = 100;
	uint32_t Y0 = 100;
	uint32_t X1 = 500;
	uint32_t Y1 = 500;
	uint32_t LineColor = 0x00ff0000;

	for(;;)
	{
		MSG Message;
		while(PeekMessageW(&Message, 0, 0, 0, PM_REMOVE))
		{
			if(Message.message == WM_QUIT)
			{
				ExitProcess(0);
			}
			DispatchMessageW(&Message);
		}
		renderer_clear(Renderer, 0x00000088);
		renderer_drawline(Renderer, X0, Y0, X1, Y1, LineColor);
		renderer_update(Renderer);
	}
}
