#if !defined(WIN32_SOFTWARE_RENDERER_H)

struct win32_back_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int BytesPerPixel;
	int Stride;
	int Width;
	int Height;
};

#define WIN32_SOFTWARE_RENDERER_H
#endif
