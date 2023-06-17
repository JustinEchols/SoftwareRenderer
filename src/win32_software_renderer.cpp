#include "software_renderer.h"

#include <windows.h>

#include "win32_software_renderer.h"
#include "software_renderer.cpp"

global_variable b32 GLOBAL_RUNNING;
global_variable win32_back_buffer Win32BackBuffer;
global_variable LARGE_INTEGER tick_frequency;
global_variable app_input AppInput;



#if 0
internal void
scan_buffer_draw_shape(win32_app_back_buffer *Win32BackBuffer, scan_buffer *ScanBuffer, f32 ymin, f32 ymax, v3f Color)
{
	// TODO(Justin): ASSERT 

	int y_min = round_f32_to_s32(ymin);
	int y_max = round_f32_to_s32(ymax);

	if (y_min < 0) {
		y_min = 0;
	}
	if (y_max >= Win32BackBuffer->height) {
		y_max = Win32BackBuffer->height - 1;
	}

	u32 color = color_convert_v3f_to_u32(Color);

	u8 *pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * y_min;
	s32 *scan_buffer_memory = (s32 *)ScanBuffer->memory;
	for (int y = y_min; y < y_max; y++) {
		int x_min = scan_buffer_memory[2 * y];
		int x_max = scan_buffer_memory[2 * y + 1];

		u8 *pixel_start = pixel_row + Win32BackBuffer->bytes_per_pixel * x_min;

		u32 *pixel = (u32 *)pixel_start;
		for (int i = x_min; i < x_max; i++) {
			*pixel++ = color;
		}
		pixel_row += Win32BackBuffer->stride;
	}
}
#endif




internal v3f
color_rand_init()
{
	v3f Result;
	Result.r = ((f32)rand() / (f32)RAND_MAX);
	Result.g = ((f32)rand() / (f32)RAND_MAX);
	Result.b = ((f32)rand() / (f32)RAND_MAX);

	return(Result);
}

internal v4f
v4f_rand_init()
{
	v4f Result = {};

	Result.x = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
	Result.y = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
	Result.z = (((f32)rand() / (f32)RAND_MAX) - 2.0f);
	Result.w = 1;

	return(Result);
}

internal triangle
triangle_rand_init()
{
	triangle Result = {};
	for(u32 i = 0; i < 3; i++) {
		Result.Vertices[i] = v4f_rand_init();
	}
	Result.Color = color_rand_init();
	return(Result);
}



internal void
win32_back_buffer_resize(win32_back_buffer *Win32BackBuffer, int width, int height)
{
	if (Win32BackBuffer->memory) {
		VirtualFree(Win32BackBuffer->memory, 0, MEM_RELEASE);
	}

	Win32BackBuffer->width = width;
	Win32BackBuffer->height = height;
	Win32BackBuffer->bytes_per_pixel = 4;
	Win32BackBuffer->stride = Win32BackBuffer->width * Win32BackBuffer->bytes_per_pixel;

	Win32BackBuffer->Info.bmiHeader.biSize = sizeof(Win32BackBuffer->Info.bmiHeader);
	Win32BackBuffer->Info.bmiHeader.biWidth = Win32BackBuffer->width;
	Win32BackBuffer->Info.bmiHeader.biHeight = Win32BackBuffer->height;
	Win32BackBuffer->Info.bmiHeader.biPlanes = 1;
	Win32BackBuffer->Info.bmiHeader.biBitCount = 32;
	Win32BackBuffer->Info.bmiHeader.biCompression = BI_RGB;


	int Win32BackBuffersize = Win32BackBuffer->width * Win32BackBuffer->height * Win32BackBuffer->bytes_per_pixel;
	Win32BackBuffer->memory = VirtualAlloc(0, Win32BackBuffersize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
}

internal LRESULT CALLBACK
win32_main_window_callback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
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

			win32_back_buffer_resize(&Win32BackBuffer, client_width, client_height);

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
			Result = DefWindowProcA(Window, Message, wParam, lParam);
		}
	}
	return(Result);
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
	win32_back_buffer_resize(&Win32BackBuffer, 960, 540);

	QueryPerformanceFrequency(&tick_frequency);
	f32 ticks_per_second = (f32)tick_frequency.QuadPart;
	f32 time_for_each_tick = 1.0f / ticks_per_second;

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

#if 0
			scan_buffer ScanBuffer;

			ScanBuffer.memory = VirtualAlloc(0, Win32BackBuffer.height * 2 * sizeof(u32), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
			ScanBuffer.height = Win32BackBuffer.height;
#endif

			GLOBAL_RUNNING = true;


			f32 time_delta = 0.0f;
			AppInput.is_initiliazed = false;

			LARGE_INTEGER tick_count_before;
			QueryPerformanceCounter(&tick_count_before);
			while (GLOBAL_RUNNING) {
				MSG Message;
				while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE)) {
					switch (Message.message) {
						case WM_KEYDOWN:
						case WM_KEYUP: 
						{
							u32 vk_code = (u32)Message.wParam;
							b32 was_down = ((Message.lParam & (1 << 30)) != 0);
							b32 is_down = ((Message.lParam & (1 << 31)) == 0);

							if (was_down != is_down) {
								switch (vk_code) {
									case 'W':
									{
										AppInput.Buttons[BUTTON_W].is_down = is_down;
									} break;
									case 'S':
									{
										AppInput.Buttons[BUTTON_S].is_down = is_down;
									} break;
									case 'A':
									{
										AppInput.Buttons[BUTTON_A].is_down = is_down;
									} break;
									case 'D':
									{
										AppInput.Buttons[BUTTON_D].is_down = is_down;
									} break;
									case VK_UP:
									{
										AppInput.Buttons[BUTTON_UP].is_down = is_down;
									} break;
									case VK_DOWN:
									{
										AppInput.Buttons[BUTTON_DOWN].is_down = is_down;
									} break;
									case VK_LEFT:
									{
										AppInput.Buttons[BUTTON_LEFT].is_down = is_down;
									} break;
									case VK_RIGHT:
									{
										AppInput.Buttons[BUTTON_RIGHT].is_down = is_down;
									} break;
									case 0x31:
									{
										AppInput.Buttons[BUTTON_1].is_down = is_down;
									} break;
									case 0x32:
									{
										AppInput.Buttons[BUTTON_2].is_down = is_down;
									} break;
									case 0x33:
									{
										AppInput.Buttons[BUTTON_3].is_down = is_down;
									} break;
								}
							}
						} break;
						default:
						{
							TranslateMessage(&Message);
							DispatchMessage(&Message);
						}
					}
				}

				app_back_buffer AppBackBuffer = {};
				AppBackBuffer.memory = Win32BackBuffer.memory;
				AppBackBuffer.bytes_per_pixel = Win32BackBuffer.bytes_per_pixel;
				AppBackBuffer.stride = Win32BackBuffer.stride;
				AppBackBuffer.width = Win32BackBuffer.width;
				AppBackBuffer.height = Win32BackBuffer.height;

				app_update_and_render(&AppBackBuffer, &AppInput);


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

				AppInput.time_delta = time_delta;
			}
		}
	}
	return(0);
}
