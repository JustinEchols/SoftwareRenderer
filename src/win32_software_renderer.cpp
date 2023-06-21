#include "software_renderer.h"

#include <windows.h>

#include "win32_software_renderer.h"

global_variable b32 GLOBAL_RUNNING;
global_variable win32_back_buffer Win32BackBuffer;
global_variable LARGE_INTEGER tick_frequency;
global_variable app_input AppInput;


typedef struct
{
	HMODULE AppDLL;
	FILETIME DLLWriteTime;

	app_update_and_render_func  *app_update_and_render;

	b32 is_valid;
} win32_app_code;

inline FILETIME
win32_file_last_write_time(char *file_name)
{
	FILETIME Result = {};
	WIN32_FIND_DATA FindData;
	HANDLE file_handle = FindFirstFileA(file_name, &FindData);
	if (file_handle != INVALID_HANDLE_VALUE) {
		Result = FindData.ftLastWriteTime;
		FindClose(file_handle);
	}
	return(Result);
}

internal win32_app_code
win32_app_code_load(char *dll_name_src, char *dll_name_tmp)
{
	win32_app_code Result = {};

	CopyFile(dll_name_src, dll_name_tmp, FALSE);
	Result.AppDLL = LoadLibraryA(dll_name_tmp);
	Result.DLLWriteTime = win32_file_last_write_time(dll_name_src);
	if (Result.AppDLL) {
		Result.app_update_and_render = (app_update_and_render_func *)
			GetProcAddress(Result.AppDLL, "app_update_and_render");
		if (Result.app_update_and_render) {
			Result.is_valid = true;
		}
	}
	return(Result);
}

DEBUG_PLATFORM_FILE_FREE(debug_platform_file_free)
{
	if (memory) {
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

internal void
win32_app_code_unload(win32_app_code *AppCode)
{
	if (AppCode->AppDLL) {
		FreeLibrary(AppCode->AppDLL);
	}
	AppCode->is_valid = false;
	AppCode->app_update_and_render = app_update_and_render_stub;
}

DEBUG_PLATFORM_FILE_READ_ENTIRE(debug_platform_file_read_entire)
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
			} else {
				// Logging
			}
		} else {
			// Logging
		}
		CloseHandle(file_handle);
	} else {
		// Logging
	}
	return(Result);
}


DEBUG_PLATFORM_FILE_WRITE_ENTIRE(debug_platform_file_write_entire)
{
	b32 Result = false;
	HANDLE file_handle = CreateFile(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
									0, 0);

	if(file_handle != INVALID_HANDLE_VALUE) {
		DWORD bytes_written;
		if(WriteFile(file_handle, memory, size, &bytes_written, 0)) {
			Result = (bytes_written == size);
		} else {
			// Logging
		}
		CloseHandle(file_handle);
	} else {
		// Logging
		DWORD Error = GetLastError();
	}
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

internal void
win32_str_cat(char *str_a, size_t str_a_size,
		      char *str_b, size_t str_b_size,
			  char *dest, size_t dest_size) 
{
	// TODO(Justin): Bounds checking.
	for (int index = 0; index < str_a_size; index++) {
		*dest++ = *str_a++;
	}

	for (int index = 0; index < str_b_size; index++) {
		*dest++ = *str_b++;
	}
	*dest++ = 0;
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	char full_path_to_exe[MAX_PATH];
	DWORD full_path_to_exe_size = GetModuleFileNameA(0, 
			full_path_to_exe, sizeof(full_path_to_exe));

	char *one_past_last_slash = full_path_to_exe;
	char *c;
	for (c = full_path_to_exe; *c != '\0'; c++) {
		if (*c == '\\') {
			one_past_last_slash = c + 1;
		}
	}
	char file_name_dll[] = "software_renderer.dll";
	char file_name_dll_tmp[] = "software_renderer_tmp.dll";

	char full_path_to_dll[MAX_PATH];
	char full_path_to_dll_tmp[MAX_PATH];

	char *dest = full_path_to_dll;
	char *str_a = full_path_to_exe;
	char *str_b = file_name_dll;


	size_t str_a_size = one_past_last_slash - full_path_to_exe;
	size_t str_b_size = sizeof(file_name_dll) - 1;
	size_t dest_size = 0;

	win32_str_cat(str_a, str_a_size, 
			      str_b, str_b_size, 
				  dest, dest_size);

	str_b = file_name_dll_tmp;
	str_b_size = sizeof(file_name_dll_tmp);

	dest = full_path_to_dll_tmp;
	dest_size = sizeof(full_path_to_dll_tmp);

	win32_str_cat(str_a, str_a_size, 
			      str_b, str_b_size, 
				  dest, dest_size);

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
#if APP_INTERNAL
			LPVOID base_address = (LPVOID)TERABYTES((u64)2);
#else
			LPVOID base_address = 0;
#endif
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

			AppMemory.debug_platform_file_free = debug_platform_file_free;
			AppMemory.debug_platform_file_read_entire = debug_platform_file_read_entire;
			AppMemory.debug_platform_file_write_entire = debug_platform_file_write_entire;


			if (AppMemory.permanent_storage && AppMemory.transient_storage) {

				win32_app_code AppCode = win32_app_code_load(full_path_to_dll, full_path_to_dll_tmp);

				GLOBAL_RUNNING = true;

				f32 time_delta = 0.0f;

				LARGE_INTEGER tick_count_before;
				QueryPerformanceCounter(&tick_count_before);
				while (GLOBAL_RUNNING) {
					//FILETIME DLLWriteTime = win32_file_last_write_time(file_name_dll);
					FILETIME DLLWriteTime = win32_file_last_write_time(full_path_to_dll);
					if (CompareFileTime(&DLLWriteTime, &AppCode.DLLWriteTime)) {
						win32_app_code_unload(&AppCode);
						AppCode = win32_app_code_load(full_path_to_dll, full_path_to_dll_tmp);
						AppCode.DLLWriteTime = DLLWriteTime;
					}
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

					AppCode.app_update_and_render(&AppBackBuffer, &AppInput, &AppMemory);


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
