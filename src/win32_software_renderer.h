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

struct win32_app_code
{
	HMODULE AppDLL;
	FILETIME DLLWriteTime;

	app_update_and_render_func  *app_update_and_render;

	b32 IsValid;
};

#define WIN32_SOFTWARE_RENDERER_H
#endif
