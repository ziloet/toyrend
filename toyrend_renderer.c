typedef struct
{
	HWND TargetWindow;
	HDC MemoryDC;
	BITMAPINFO Bitmap;
	uint32_t* PixelData;
} gdi_renderer;

static void
renderer_resize(gdi_renderer* Renderer)
{
	RECT WindowRect;
	GetClientRect(Renderer->TargetWindow, &WindowRect);
	LONG NewWidth = WindowRect.right - WindowRect.left;
	LONG NewHeight = WindowRect.bottom - WindowRect.top;
	Renderer->Bitmap.bmiHeader.biWidth = NewWidth;
	Renderer->Bitmap.bmiHeader.biHeight = NewHeight;
	size_t AllocSize = sizeof(*Renderer->PixelData) * NewWidth * NewHeight;
	Renderer->PixelData = VirtualAlloc(Renderer->PixelData, AllocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	Assert(Renderer->PixelData);	
}

static gdi_renderer*
renderer_create(HWND Window)
{
	gdi_renderer* Result = VirtualAlloc(0, sizeof(*Result), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	Assert(Result);
	Result->TargetWindow = Window;
	renderer_resize(Result);
	HDC WindowDC = GetDC(Window);
	Result->MemoryDC = CreateCompatibleDC(WindowDC);
	Result->Bitmap.bmiHeader.biSize = sizeof(Result->Bitmap.bmiHeader);
	Result->Bitmap.bmiHeader.biPlanes = 1;
	Result->Bitmap.bmiHeader.biBitCount = 32;
	Result->Bitmap.bmiHeader.biCompression = BI_RGB;
	return Result;
}

static void
renderer_clear(gdi_renderer* Renderer, uint32_t ClearColor)
{
	LONG BufferWidth = Renderer->Bitmap.bmiHeader.biWidth;
	LONG BufferHeight = Renderer->Bitmap.bmiHeader.biHeight;
	size_t BufferSize = BufferWidth * BufferHeight;
	for(size_t Index = 0; Index < BufferSize; Index += 1)
	{
		Renderer->PixelData[Index] = ClearColor;
	}
}

static void
renderer_drawpixel(gdi_renderer* Renderer, LONG X, LONG Y, uint32_t Color)
{
	LONG BufferWidth = Renderer->Bitmap.bmiHeader.biWidth;
	LONG BufferHeight = Renderer->Bitmap.bmiHeader.biHeight;
	int OutOfBounds = (X >= BufferWidth) | (Y >= BufferHeight);
	return_if(OutOfBounds);
	Renderer->PixelData[X + (Y * BufferWidth)] = Color;
}
