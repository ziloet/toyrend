typedef struct
{
	HWND TargetWindow;
	HDC MemoryDC;
	BITMAPINFO Bitmap;
	HBITMAP BitmapHandle;
	uint32_t* PixelData;
} gdi_renderer;

static void
renderer_resize(gdi_renderer* Renderer)
{
	RECT WindowRect;
	if(Renderer->PixelData)
	{
		VirtualFree(Renderer->PixelData, 0, MEM_RELEASE);
	}
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

static void
renderer_drawline(gdi_renderer* Renderer, int X0, int Y0, int X1, int Y1, uint32_t Color)
{
	int DeltaX = X1 - X0;
	int DeltaY = Y1 - Y0;
	int StepX = sign(DeltaX);
	int StepY = sign(DeltaY);
	int Error = DeltaX + DeltaY;
	int X = X0;
	int Y = Y0;

	for(;;)
	{
		renderer_drawpixel(Renderer, X, Y, Color);
		int Done = ((X == X1) & (Y == Y1));
		return_if(Done);

		int Error2 = Error * 2;

		if(Error2 >= DeltaY)
		{
			X += StepX;
			Error += DeltaY;
		}
		if(Error2 >= DeltaX)
		{
			Y += StepY;
			Error += DeltaX;
		}
	}
}

static void
renderer_drawcircle(gdi_renderer* Renderer, uint32_t PositionX, uint32_t PositionY, uint32_t Radius, uint32_t Color)
{
	int F = 1 - Radius;
	int ddF_X = 0;
	int ddF_Y = -2 * Radius;
	int X = 0;
	int Y = Radius;

	renderer_drawpixel(Renderer, PositionX, PositionY + Radius, Color);
	renderer_drawpixel(Renderer, PositionX, PositionY - Radius, Color);
	renderer_drawpixel(Renderer, PositionX + Radius, PositionY, Color);
	renderer_drawpixel(Renderer, PositionX - Radius, PositionY, Color);

	while(X < Y)
	{
		if(F >= 0)
		{
			Y -= 1;
			ddF_Y += 2;
			F += ddF_Y;
		}
		X += 1;
		ddF_X += 2;
		F += ddF_X + 1;
		renderer_drawpixel(Renderer, PositionX + X, PositionY + Y, Color);
		renderer_drawpixel(Renderer, PositionX - X, PositionY + Y, Color);
		renderer_drawpixel(Renderer, PositionX + X, PositionY - Y, Color);
		renderer_drawpixel(Renderer, PositionX - X, PositionY - Y, Color);
		renderer_drawpixel(Renderer, PositionX + Y, PositionY + X, Color);
		renderer_drawpixel(Renderer, PositionX - Y, PositionY + X, Color);
		renderer_drawpixel(Renderer, PositionX + Y, PositionY - X, Color);
		renderer_drawpixel(Renderer, PositionX - Y, PositionY - X, Color);
	}
}

static void
renderer_update(gdi_renderer* Renderer)
{
	HDC WindowDC = GetDC(Renderer->TargetWindow);
	int Width = Renderer->Bitmap.bmiHeader.biWidth;
	int Height = Renderer->Bitmap.bmiHeader.biHeight;
	StretchDIBits(WindowDC, 0, 0, Width, Height, 0, 0, Width, Height, Renderer->PixelData, &Renderer->Bitmap, DIB_RGB_COLORS, SRCCOPY);
	ReleaseDC(Renderer->TargetWindow, WindowDC);
}
