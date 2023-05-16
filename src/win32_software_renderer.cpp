//
// NOTE(Justin): How far along do we need to move along b for every step along a? a / b
// slope...
//
//    /|
//   / |
//  /  | a
// /___|
//   b
//

// NOTE(Justin): For scan conversion rasterization, the shape that is to
// be drawn is defined by the set of min/max x values in the scan buffer.
// As well as the the starting and ending y values (the number of scanlines).
//
// E.g. Suppose we want to draw a square. The minimum and maximum values for
// each entry in the scan buffer are constants. That is, if we wanted to draw a square
// at the bottom left corner that is 100 pixels wide, then for each entry in 
// the scan buffer, the minimum is always 0 and the maximum is always 99. Furthermore, 
// for the shape to actually be a square, the number of scanlines would be 100.
// As we would have 100 scanlines and each scanline is 100 pixels wide. So,
// the dimensions are 100 by 100. 
//
// Other shapes are defined by variying entries in the scan buffer, according
// to the desired geometry. 

#include <windows.h>

#include "software_renderer_types.h"
#include "software_renderer_math.h"

global_variable b32 GLOBAL_RUNNING;
global_variable win32_back_buffer Win32BackBuffer;
global_variable LARGE_INTEGER tick_frequency;


internal void
pixel_set(win32_back_buffer *Win32BackBuffer, v2f pixel_position, v3f color)
{
	int x = (int)(pixel_position.x + 0.5f);
	int y = (int)(pixel_position.y + 0.5f);

	if (x < 0) {
		x = 0;
	}
	if (x >= Win32BackBuffer->width) {
		x = Win32BackBuffer->width;
	}
	if (y < 0) {
		y = 0;
	}
	if (y >= Win32BackBuffer->height) {
		y = Win32BackBuffer->height;
	}

	u32 red = (u32)(255.0f * color.r);
	u32 green = (u32)(255.0f * color.g);
	u32 blue = (u32)(255.0f * color.b);
	u32 color_value = ((red << 16) | (green << 8) | (blue << 0));

	u8 *pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * y + Win32BackBuffer->bytes_per_pixel * x;
	u32 *pixel = (u32 *)pixel_row;
	*pixel = color_value;
}

internal void
scan_buffer_fill_triangle(u32 *scan_buffer, triangle *Triangle)
{
	v3f tmp = {};
	v3f *min = &Triangle->Vertices[0];
	v3f *mid = &Triangle->Vertices[1];
	v3f *max = &Triangle->Vertices[2];

	if (min->y > max->y) {
		tmp = *min;
		*min = *max;
		*max = tmp;
	}
	if (mid->y >  max->y) {
		tmp = *mid;
		*mid = *max;
		*max = tmp;
	}
	if (min->y > mid->y) {
		tmp = *min;
		*min = *mid;
		*mid = tmp;
	}

	// NOTE(Justin): To determine whether or not the triangle is right oriented 
	// or left oriented, find the signed area of the triangle using the cross product
	// of two vectors which represent two sides of the triangle. The first vector
	// is the vector FROM MAX TO MIN. The second is FROM MAX TO MID. Note that
	// we are using the sign to determine the orientation of the triangle.
	
	f32 v0_x = min->x - max->x;
	f32 v0_y = min->y - max->y;

	f32 v1_x = mid->x - max->x;
	f32 v1_y = mid->y - max->y;

	f32 area_double_signed = v0_x * v1_y - v1_x * v0_y;

	b32 oriented_right;
	if (area_double_signed > 0) {
		oriented_right = true;
	} else {
		oriented_right = false;
	}

	b32 which_side;
	if (oriented_right) {
		which_side = false;
	} else {
		which_side = true;
	}

	u32 y_start = (u32)(Triangle->Vertices[0].y);
	u32 y_end = (u32)(Triangle->Vertices[2].y);
	u32 y_dist = y_end - y_start;

	u32 x_start = (u32)(Triangle->Vertices[0].x);
	u32 x_end = (u32)(Triangle->Vertices[2].x);
	s32 x_dist = x_end - x_start;

	f32 x_step = (f32)x_dist / (f32)y_dist;
	f32 x_current = (f32)x_start;

	for (u32 j = y_start; j < y_end; j++) {
		scan_buffer[2 * j + which_side] = (u32)x_current;
		x_current += x_step;
	}

	//
	// NOTE(Justin): Flip which side
	//

	which_side = !which_side;

	y_start = (u32)Triangle->Vertices[1].y;
	y_end = (u32)Triangle->Vertices[2].y;
	y_dist = y_end - y_start;

	x_start = (u32)Triangle->Vertices[1].x;
	x_end = (u32)Triangle->Vertices[2].x;
	x_dist = x_end - x_start;

	x_step = (f32)x_dist / (f32)y_dist;
	x_current = (f32)x_start;


	for (u32 j = y_start; j < y_end; j++) {
		scan_buffer[2 * j + which_side] = (u32)x_current;
		x_current += x_step;
	}

	y_start = (u32)Triangle->Vertices[0].y;
	y_end = (u32)Triangle->Vertices[1].y;
	y_dist = y_end - y_start;

	x_start = (u32)Triangle->Vertices[0].x;
	x_end = (u32)Triangle->Vertices[1].x;
	x_dist = x_end - x_start;

	x_step = (f32)x_dist / (f32)y_dist;
	x_current = (f32)x_start;

	for (u32 j = y_start; j < y_end; j++) {
		scan_buffer[2 * j + which_side] = (u32)x_current;
		x_current += x_step;
	}
}


internal void
scan_buffer_draw_shape(win32_back_buffer *Win32BackBuffer, u32 *scan_buffer, int y_min, int y_max)
{
	// TODO(Justin): ASSERT 

	
	u8 *pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * y_min;
	for (int y = y_min; y < y_max; y++) {
		int x_min = scan_buffer[2 * y];
		int x_max = scan_buffer[2 * y + 1];

		u8 *pixel_start = pixel_row + Win32BackBuffer->bytes_per_pixel * x_min;

		u32 *pixel = (u32 *)pixel_start;
		for (int i = x_min; i < x_max; i++) {
			*pixel++ = 0xFFFFFFFF;
		}
		pixel_row += Win32BackBuffer->stride;
	}
}

internal triangle
triangle_rand_init()
{
	triangle Result = {};
	for(u32 i = 0; i < 3; i++) {
		Result.Vertices[i].x = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
		Result.Vertices[i].y = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
		Result.Vertices[i].z = ((((f32)rand() / (f32)RAND_MAX) * 1.0f) - 0.00001f);
	}
	return(Result);

}

internal m4x4
m4x4_screen_space_map_create(win32_back_buffer *Win32BackBuffer)
{
	m4x4 R;

	v3f x_scale_to_screen = v3f_create_from_scalars((f32)Win32BackBuffer->width / 2.0f, 1.0f, 1.0f);
	v3f x_shift_to_screen = v3f_create_from_scalars((f32)Win32BackBuffer->width / 2.0f, 0.0f, 0.0f);

	// WARNING(Justin): The order here matters. If we first shift then scale,
	// this produces an undesired result. Since matrix multiplication is from
	// right to left, the rightmost matrix must be the scaling transformation.
	// Then we apply the shifting transformation.

	m4x4 Scale_x = m4x4_scale_create(x_scale_to_screen);
	m4x4 Shift_x = m4x4_translation_create(x_shift_to_screen);
	m4x4 Map_x = Shift_x * Scale_x;

	v3f y_scale_to_screen = v3f_create_from_scalars(1.0f, (f32)Win32BackBuffer->height / 2.0f, 1.0f);
	v3f y_shift_to_screen = v3f_create_from_scalars(0.0f, (f32)Win32BackBuffer->height / 2.0f, 0.0f);

	m4x4 Scale_y = m4x4_scale_create(y_scale_to_screen);
	m4x4 Shift_y = m4x4_translation_create(y_shift_to_screen);
	m4x4 Map_y = Shift_y * Scale_y;

	R = Map_y * Map_x;

	return(R);
}

internal LRESULT CALLBACK
win32_main_window_callback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (Message) {
		case WM_CLOSE:
		case WM_DESTROY:
			{
				GLOBAL_RUNNING = false;
			} break;
		case WM_SIZE:
			{
				RECT Rect;
				GetClientRect(Window, &Rect);

				int client_width = Rect.right - Rect.left;
				int client_height = Rect.bottom - Rect.top;

				Win32BackBuffer.width = client_width;
				Win32BackBuffer.height = client_height;
				Win32BackBuffer.stride = Win32BackBuffer.width * Win32BackBuffer.bytes_per_pixel;

				Win32BackBuffer.Info.bmiHeader.biSize = sizeof(Win32BackBuffer.Info.bmiHeader);
				Win32BackBuffer.Info.bmiHeader.biWidth = Win32BackBuffer.width;
				Win32BackBuffer.Info.bmiHeader.biHeight = Win32BackBuffer.height;
				Win32BackBuffer.Info.bmiHeader.biPlanes = 1;
				Win32BackBuffer.Info.bmiHeader.biBitCount = 32;
				Win32BackBuffer.Info.bmiHeader.biCompression = BI_RGB;

				if (Win32BackBuffer.memory) {
					VirtualFree(Win32BackBuffer.memory, 0, MEM_RELEASE);
				}

				int Win32BackBuffersize = Win32BackBuffer.width * Win32BackBuffer.height * Win32BackBuffer.bytes_per_pixel;
				Win32BackBuffer.memory = VirtualAlloc(0, Win32BackBuffersize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);

			} break;
		case WM_PAINT:
			{
				PAINTSTRUCT PaintStruct;
				HDC DeviceContext = BeginPaint(Window, &PaintStruct);

				StretchDIBits(DeviceContext,
						0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
						0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
						Win32BackBuffer.memory,
						&Win32BackBuffer.Info,
						DIB_RGB_COLORS,
						SRCCOPY);

				EndPaint(Window, &PaintStruct);

			} break;
		default:
			{
				result = DefWindowProcA(Window, Message, wParam, lParam);
			}
	}
	return(result);
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSA WindowClass = {};

	WindowClass.style = (CS_HREDRAW | CS_VREDRAW);
	WindowClass.lpfnWndProc = win32_main_window_callback;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = "Main window";

	Win32BackBuffer = {};
	Win32BackBuffer.width = 960;
	Win32BackBuffer.height = 540;
	Win32BackBuffer.bytes_per_pixel = 4;
	Win32BackBuffer.stride = Win32BackBuffer.width * Win32BackBuffer.bytes_per_pixel;

	Win32BackBuffer.Info.bmiHeader.biSize = sizeof(Win32BackBuffer.Info.bmiHeader);
	Win32BackBuffer.Info.bmiHeader.biWidth = Win32BackBuffer.width;
	Win32BackBuffer.Info.bmiHeader.biHeight = Win32BackBuffer.height;
	Win32BackBuffer.Info.bmiHeader.biPlanes = 1;
	Win32BackBuffer.Info.bmiHeader.biBitCount = 32;
	Win32BackBuffer.Info.bmiHeader.biCompression = BI_RGB;

	QueryPerformanceFrequency(&tick_frequency);
	f32 ticks_per_second = (f32)tick_frequency.QuadPart;
	f32 time_for_each_tick = 1.0f / ticks_per_second;

	u32 *scan_buffer = (u32 *)VirtualAlloc(0, Win32BackBuffer.height * 2 * sizeof(u32), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);

	srand(2023);

	triangle Triangle = triangle_rand_init();
	m4x4 MapToScreenSpace = m4x4_screen_space_map_create(&Win32BackBuffer);


	for(u32 i = 0; i < 3; i++) {
		Triangle.Vertices[i] = MapToScreenSpace * Triangle.Vertices[i];
	}

	scan_buffer_fill_triangle(scan_buffer, &Triangle);

	f32 direction = 1.0f;


	if (RegisterClassA(&WindowClass)) {
		HWND Window = CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Rasterization Test",
				(WS_OVERLAPPEDWINDOW | WS_VISIBLE),
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				0,
				0,
				hInstance,
				0);

		if (Window) {
			HDC DeviceContext = GetDC(Window);

			GLOBAL_RUNNING = true;

			int scan_buffer_height = Win32BackBuffer.height;

			f32 time_delta = 0.0f;

			LARGE_INTEGER tick_count_before;
			QueryPerformanceCounter(&tick_count_before);
			while (GLOBAL_RUNNING) {
				MSG Message;
				while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE)) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

				// Clear screen to black
				u32 *pixel = (u32 *)Win32BackBuffer.memory;
				for (s32 y = 0; y < Win32BackBuffer.height; y++) {
					for (s32 x = 0; x < Win32BackBuffer.width; x++) {
						*pixel++ = 0;
					}
				}

				// Resize the scan buffer upon a change in the window size
				if (scan_buffer_height != Win32BackBuffer.height) {
					if (scan_buffer) {
						VirtualFree(scan_buffer, 0, MEM_RELEASE);
					}
					scan_buffer = (u32 *)VirtualAlloc(0, Win32BackBuffer.height * 2 * sizeof(u32), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
					scan_buffer_height = Win32BackBuffer.height;

					MapToScreenSpace = m4x4_screen_space_map_create(&Win32BackBuffer);
				}

				// 2D Transformations

				v3f shift = v3f_create_from_scalars(time_delta * 200.0f, time_delta * 200.0f, 0.0f);
				shift = direction * shift;

				m4x4 Translate = m4x4_translation_create(shift);

				for (u32 i = 0; i < 3; i++) {

					Triangle.Vertices[i] = Translate * Triangle.Vertices[i];
					if ((Triangle.Vertices[i].x < 0) || (Triangle.Vertices[i].x >= Win32BackBuffer.width) ||
						(Triangle.Vertices[i].y < 0) || (Triangle.Vertices[i].y >= Win32BackBuffer.height)) {

						Triangle = triangle_rand_init();
						for (u32 i = 0; i < 3; i++) {
							Triangle.Vertices[i] = MapToScreenSpace * Triangle.Vertices[i];
						}
						int choice = rand();
						if (choice < RAND_MAX / 2) {
							direction = -1.0f;
						} else {
							direction = 1.0f;
						}
						break;
					}
				}
				scan_buffer_fill_triangle(scan_buffer, &Triangle);

				// NOTE(Justin): The routine above contains in it a sorting
				// block that sorts the veritces of the triangles based on the y values. 
				// The vertices are sorted from least to greatest y values. Therefore since the fill shape
				// routing accepts a y_min and y_max, we pass the y value of the
				// first v3f as the y min and the y value of the third v3f
				// as the y max.


				// NOTE(Justin): This routine assumes that the scan buffer has
				// been populated with the necessary min and max x values such
				// that the geometry may be rendered.

				// TODO(Justin): Pass floats then round inside routine
				scan_buffer_draw_shape(&Win32BackBuffer, scan_buffer, (int)Triangle.Vertices[0].y, (int)Triangle.Vertices[2].y);

				// Blit
				StretchDIBits(DeviceContext,
						0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
						0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
						Win32BackBuffer.memory,
						&Win32BackBuffer.Info,
						DIB_RGB_COLORS,
						SRCCOPY);

				LARGE_INTEGER tick_count_after;
				QueryPerformanceCounter(&tick_count_after);

				f32 tick_count_elapsed = (f32)(tick_count_after.QuadPart - tick_count_before.QuadPart);
				time_delta = tick_count_elapsed * time_for_each_tick;
				tick_count_before = tick_count_after;
			}
		}
	}
	return(0);
}

