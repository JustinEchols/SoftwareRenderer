#if !defined(WIN32_SOFTWARE_RENDERER_H)

typedef struct
{
	BITMAPINFO Info;
	void *memory;
	int bytes_per_pixel;
	int stride;
	int width;
	int height;

} win32_back_buffer;

typedef struct
{
	void *memory;
	int height;
} scan_buffer;

#define WIN32_SOFTWARE_RENDERER_H
#endif
