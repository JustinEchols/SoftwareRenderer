#include "software_renderer.h"

#include <windows.h>

#include "win32_software_renderer.h"
#include "software_renderer.cpp"

global_variable b32 GLOBAL_RUNNING;
global_variable win32_back_buffer Win32BackBuffer;
global_variable LARGE_INTEGER tick_frequency;
global_variable app_input AppInput;


#if 1
internal debug_file_read_result
debug_platform_file_read_entire(char *file_name)
{
	debug_file_read_result Result = {};
	HANDLE file_handle = CreateFileA(file_name,
									GENERIC_READ,
									FILE_SHARE_READ,
									0, OPEN_EXISTING,
									0,0);

	DWORD LastError = GetLastError();
	if (file_handle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(file_handle, &file_size)) {

			u32 file_size32 = u64_truncate_to_u32(file_size.QuadPart);
			Result.memory = VirtualAlloc(0, file_size32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (Result.memory) {

				DWORD bytes_read;
				if (ReadFile(file_handle, Result.memory, file_size32, &bytes_read, 0) && (file_size32 == bytes_read)) {
					Result.size = file_size32;
				} else {
					debug_platform_file_free(Result.memory);
					Result.size = 0;
				}
			}
		}
	}
	return(Result);
}


internal void
debug_platform_file_free(void *memory)
{
	if (memory) {
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}


internal b32 
debug_platform_file_write_entire(char *file_name, u32 size, void *memory)
{
}
#endif

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
	debug_file_read_result file = debug_platform_file_read_entire("..\\src\\scratch.txt");

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


			LPVOID base_address;
#if APP_INTERNAL
			base_address = (LPVOID)TERABYTES((u64)2);
#else
			base_address = 0;
#endif
			// TODO(Justin): Win32State..


			app_memory AppMemory = {};
			AppMemory.permanent_storage_size = MEGABYTES((u64)64);
			AppMemory.transient_storage_size = GIGABYTES((u64)4);
			u64 total_size = AppMemory.permanent_storage_size + AppMemory.transient_storage_size;


			AppMemory.permanent_storage = VirtualAlloc(base_address, 
					(size_t)total_size, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);

			// NOTE(Justin):
			// The starting address of the transient storage is 64kb into the
			// memory block just allocated. Which is exactly the size of the
			// permanent storage. Therefore if we get a pointer 64KB into the
			// permananet storage memory block this pointer points to the first
			// byte "outside" this block which is where we have decided to use
			// as transient storage.
			AppMemory.transient_storage = ((u8 *)AppMemory.permanent_storage + AppMemory.permanent_storage_size);

			if (AppMemory.permanent_storage && AppMemory.transient_storage) {

				GLOBAL_RUNNING = true;

				f32 time_delta = 0.0f;

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

					app_update_and_render(&AppBackBuffer, &AppInput, &AppMemory);


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
	}
	return(0);
}
