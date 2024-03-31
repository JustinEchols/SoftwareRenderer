#include "software_renderer.h"

#include <windows.h>

#include "win32_software_renderer.h"

global_variable b32 Win32GlobalRunning;
global_variable win32_back_buffer Win32GlobalBackBuffer;
global_variable LARGE_INTEGER Win32GlobalTickFrequency;

// NOTE(Justin): These are global for now
global_variable f32 dMouseX;
global_variable f32 dMouseY;

internal void
Win32ClipCursorToClientArea(HWND Window)
{
	RECT ClipRect;
	GetClientRect(Window, &ClipRect);
	ClientToScreen(Window, (POINT *)&ClipRect.left);
	ClientToScreen(Window, (POINT *)&ClipRect.right);
	ClipCursor(&ClipRect);
}

internal void
Win32CenterCursor(HWND Window)
{
	RECT WindowRect;
	GetWindowRect(Window, &WindowRect);
	u32 CenterX = WindowRect.left + (Win32GlobalBackBuffer.Width / 2);
	u32 CenterY = WindowRect.top + (Win32GlobalBackBuffer.Height / 2);

	RECT SnapPosRect;
	SnapPosRect.left = (s32)CenterX;
	SnapPosRect.right = (s32)CenterX;
	SnapPosRect.top = (s32)CenterY;
	SnapPosRect.bottom = (s32)CenterY;
	ClipCursor(&SnapPosRect);
}

internal POINT
Win32CursorPosRelativeToClient(HWND Window)
{
	POINT Result;

	GetCursorPos(&Result);
	ScreenToClient(Window, &Result);

	return(Result);
}

typedef struct
{
	HMODULE AppDLL;
	FILETIME DLLWriteTime;

	app_update_and_render_func  *app_update_and_render;

	b32 is_valid;
} win32_app_code;

inline FILETIME
Win32FileLastWriteTime(char *FileName)
{
	FILETIME Result = {};
	WIN32_FIND_DATA FindData;
	HANDLE FileHandle = FindFirstFileA(FileName, &FindData);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		Result = FindData.ftLastWriteTime;
		FindClose(FileHandle);
	}
	return(Result);
}

internal win32_app_code
Win32AppCodeLoad(char *DllNameSrc, char *DllNameTmp)
{
	win32_app_code Result = {};

	CopyFile(DllNameSrc, DllNameTmp, FALSE);
	Result.AppDLL = LoadLibraryA(DllNameTmp);
	Result.DLLWriteTime = Win32FileLastWriteTime(DllNameSrc);
	if(Result.AppDLL)
	{
		Result.app_update_and_render = (app_update_and_render_func *)
			GetProcAddress(Result.AppDLL, "app_update_and_render");
		if(Result.app_update_and_render)
		{
			Result.is_valid = true;
		}
	}
	return(Result);
}

DEBUG_PLATFORM_FILE_FREE(debug_platform_file_free)
{
	if(Memory) {
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

internal void
Win32AppCodeUnload(win32_app_code *AppCode)
{
	if(AppCode->AppDLL) {
		FreeLibrary(AppCode->AppDLL);
	}
	AppCode->is_valid = false;
	AppCode->app_update_and_render = app_update_and_render_stub;
}

DEBUG_PLATFORM_FILE_READ_ENTIRE(debug_platform_file_read_entire)
{
	debug_file_read_result Result = {};
	HANDLE FileHandle = CreateFileA(FileName,
									GENERIC_READ,
									FILE_SHARE_READ,
									0, OPEN_EXISTING,
									0,0);

	DWORD LastError = GetLastError();
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{

			u32 FileSize32 = U64TruncateToU32(FileSize.QuadPart);
			Result.Memory = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(Result.Memory)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Memory, FileSize32, &BytesRead, 0) &&
													  (FileSize32 == BytesRead))
				{
					Result.Size = FileSize32;
				}
				else
				{
					debug_platform_file_free(Thread, Result.Memory);
					Result.Size = 0;
				}
			}
			else
			{
				// Logging
			}
		}
		else
		{
			// Logging
		}

		CloseHandle(FileHandle);
	}
	else
	{
		// Logging
	}

	return(Result);
}


DEBUG_PLATFORM_FILE_WRITE_ENTIRE(debug_platform_file_write_entire)
{
	b32 Result = false;
	HANDLE FileHandle = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
									0, 0);

	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory, Size, &BytesWritten, 0))
		{
			Result = (BytesWritten == Size);
		}
		else
		{
			// Logging
		}

		CloseHandle(FileHandle);
	}
	else
	{
		// Logging
		DWORD Error = GetLastError();
	}

	return(Result);
}

internal void
Win32BackBufferReSize(win32_back_buffer *Win32GlobalBackBuffer, int Width, int Height)
{
	if(Win32GlobalBackBuffer->Memory)
	{
		VirtualFree(Win32GlobalBackBuffer->Memory, 0, MEM_RELEASE);
	}

	Win32GlobalBackBuffer->Width = Width;
	Win32GlobalBackBuffer->Height = Height;
	Win32GlobalBackBuffer->BytesPerPixel = 4;
	Win32GlobalBackBuffer->Stride = Win32GlobalBackBuffer->Width * Win32GlobalBackBuffer->BytesPerPixel;

	Win32GlobalBackBuffer->Info.bmiHeader.biSize = sizeof(Win32GlobalBackBuffer->Info.bmiHeader);
	Win32GlobalBackBuffer->Info.bmiHeader.biWidth = Win32GlobalBackBuffer->Width;
	Win32GlobalBackBuffer->Info.bmiHeader.biHeight = Win32GlobalBackBuffer->Height;
	Win32GlobalBackBuffer->Info.bmiHeader.biPlanes = 1;
	Win32GlobalBackBuffer->Info.bmiHeader.biBitCount = 32;
	Win32GlobalBackBuffer->Info.bmiHeader.biCompression = BI_RGB;


	int Win32GlobalBackBufferSize = Win32GlobalBackBuffer->Width * Win32GlobalBackBuffer->Height * Win32GlobalBackBuffer->BytesPerPixel;
	Win32GlobalBackBuffer->Memory = VirtualAlloc(0, Win32GlobalBackBufferSize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
}


internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_CLOSE:
		{
		} break;
		case WM_DESTROY:
		{
			Win32GlobalRunning = false;
		} break;
		case WM_ACTIVATEAPP:
		{
			Win32CenterCursor(Window);
		} break;
		case WM_SIZE:
		{
			RECT Rect;
			GetClientRect(Window, &Rect);

			int client_Width = Rect.right - Rect.left;
			int client_Height = Rect.bottom - Rect.top;

			Win32BackBufferReSize(&Win32GlobalBackBuffer, client_Width, client_Height);

		} break;
		case WM_MOVE:
		{

		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT PaintStruct;
			HDC DeviceContext = BeginPaint(Window, &PaintStruct);

			StretchDIBits(DeviceContext,
					0, 0, Win32GlobalBackBuffer.Width, Win32GlobalBackBuffer.Height,
					0, 0, Win32GlobalBackBuffer.Width, Win32GlobalBackBuffer.Height,
					Win32GlobalBackBuffer.Memory,
					&Win32GlobalBackBuffer.Info,
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
StrCat(char *StrA, size_t StrASize,
	   char *StrB, size_t StrBSize,
	   char *Dest, size_t DestSize) 
{
	// TODO(Justin): Bounds checking.
	for(int Index = 0; Index < StrASize; Index++) {
		*Dest++ = *StrA++;
	}

	for(int Index = 0; Index < StrBSize; Index++) {
		*Dest++ = *StrB++;
	}
	*Dest++ = 0;
}

internal void
Win32ProcessKeyboardMessage(app_button_state *NewState, b32 IsDown)
{
	if(NewState->EndedDown != IsDown)
	{
		NewState->EndedDown = IsDown;
		++NewState->HalfTransitionCount;
	}
}

internal void
Win32ProcessPendingMessages(app_input *Input, app_keyboard_controller *KeyboardController)
{

	MSG Message;
	while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		switch(Message.message)
		{
			case WM_KEYDOWN:
			case WM_KEYUP: 
			{
				u32 vk_code = (u32)Message.wParam;
				b32 was_down = ((Message.lParam & (1 << 30)) != 0);
				b32 is_down = ((Message.lParam & (1 << 31)) == 0);

				if(was_down != is_down)
				{
					switch(vk_code)
					{
						case 'W':
						{
							Win32ProcessKeyboardMessage(&KeyboardController->W, is_down);
						} break;
						case 'S':
						{
							Win32ProcessKeyboardMessage(&KeyboardController->S, is_down);
						} break;
						case 'A':
						{
							Win32ProcessKeyboardMessage(&KeyboardController->A, is_down);
						} break;
						case 'D':
						{
							Win32ProcessKeyboardMessage(&KeyboardController->D, is_down);
						} break;
						case VK_UP:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->Up, is_down);
						} break;
						case VK_DOWN:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->Down, is_down);
						} break;
						case VK_LEFT:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->Left, is_down);
						} break;
						case VK_RIGHT:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->Right, is_down);
						} break;
						case 0x31:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->One, is_down);
						} break;
						case 0x32:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->Two, is_down);
						} break;
						case 0x33:
						{
							Win32ProcessKeyboardMessage(&KeyboardController->Three, is_down);
						} break;
					}
				}
			} break;
			case WM_INPUT:
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse

				UINT dwSize = sizeof(RAWINPUT);
				static BYTE lpb[sizeof(RAWINPUT)];

				GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

				RAWINPUT* RawInput = (RAWINPUT*)lpb;

				if (RawInput->header.dwType == RIM_TYPEMOUSE) 
				{
					// TODO(Justin): Remove global
					dMouseX += RawInput->data.mouse.lLastX;
					dMouseY -= RawInput->data.mouse.lLastY;
				}

			} break;
			default:
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
	}
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	char FullPathToExe[MAX_PATH];
	DWORD FullPathToExeSize = GetModuleFileNameA(0, 
			FullPathToExe, sizeof(FullPathToExe));

	char *OnePastLastSlash = FullPathToExe;
	char *c;
	for(c = FullPathToExe; *c != '\0'; c++) {
		if(*c == '\\') {
			OnePastLastSlash = c + 1;
		}
	}
	char FileNameDll[] = "software_renderer.dll";
	char FileNameDllTmp[] = "software_renderer_tmp.dll";

	char FullPathToDll[MAX_PATH];
	char FullPathToDllTmp[MAX_PATH];

	char *Dest = FullPathToDll;
	char *StrA = FullPathToExe;
	char *StrB = FileNameDll;

	size_t StrASize = OnePastLastSlash - FullPathToExe;
	size_t StrBSize = sizeof(FileNameDll) - 1;
	size_t DestSize = 0;

	StrCat(StrA, StrASize, 
		   StrB, StrBSize, 
		   Dest, DestSize);

	StrB = FileNameDllTmp;
	StrBSize = sizeof(FileNameDllTmp);

	Dest = FullPathToDllTmp;
	DestSize = sizeof(FullPathToDllTmp);

	StrCat(StrA, StrASize, StrB, StrBSize, Dest, DestSize);

	WNDCLASSEX WindowClass = {};

	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = (CS_HREDRAW | CS_VREDRAW);
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = hInstance;
	WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WindowClass.lpszClassName = "Main window";

	Win32GlobalBackBuffer = {};
	Win32BackBufferReSize(&Win32GlobalBackBuffer, 960, 540);

	QueryPerformanceFrequency(&Win32GlobalTickFrequency);
	f32 TicksPerSecond = (f32)Win32GlobalTickFrequency.QuadPart;
	f32 TimeForEachTick = 1.0f / TicksPerSecond;

	if(RegisterClassEx(&WindowClass))
	{
		HWND Window = CreateWindowEx(0,
									WindowClass.lpszClassName,
									"Software Renderer",
									(WS_OVERLAPPEDWINDOW | WS_VISIBLE),
									CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
									0,
									0,
									hInstance,
									0);

		if(Window)
		{
			HDC DeviceContext = GetDC(Window);
#if APP_INTERNAL
			LPVOID BaseAddress = (LPVOID)TERABYTES((u64)2);
#else
			LPVOID BaseAddress = 0;
#endif

			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = ((USHORT) 0x01); 
			Rid[0].usUsage = ((USHORT) 0x02); 
			Rid[0].dwFlags = RIDEV_INPUTSINK;   
			Rid[0].hwndTarget = Window;
			RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

			ShowCursor(false);
			Win32CenterCursor(Window);

			app_memory AppMemory = {};
			AppMemory.PermanentStorageSize = MEGABYTES((u64)64);
			AppMemory.TransientStorageSize = GIGABYTES((u64)4);
			u64 TotalSize = AppMemory.PermanentStorageSize + AppMemory.TransientStorageSize;

			AppMemory.PermanentStorage = VirtualAlloc(BaseAddress, 
					(size_t)TotalSize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);

			// NOTE(Justin):
			// The starting address of the transient storage is 64kb into the
			// Memory block just allocated. Which is exactly the Size of the
			// permanent storage. Therefore if we get a pointer 64KB into the
			// permananet storage Memory block this pointer points to the first
			// byte "outside" this block which is where we have decided to use
			// as transient storage.

			AppMemory.TransientStorage = ((u8 *)AppMemory.PermanentStorage + AppMemory.PermanentStorageSize);

			AppMemory.debug_platform_file_free = debug_platform_file_free;
			AppMemory.debug_platform_file_read_entire = debug_platform_file_read_entire;
			AppMemory.debug_platform_file_write_entire = debug_platform_file_write_entire;


			if(AppMemory.PermanentStorage && AppMemory.TransientStorage)
			{
				win32_app_code AppCode = Win32AppCodeLoad(FullPathToDll, FullPathToDllTmp);

				app_input AppInput[2] = {};
				app_input *NewInput = &AppInput[0];
				app_input *OldInput = &AppInput[1];

				f32 dtForFrame = 0.0f;

				Win32GlobalRunning = true;
				LARGE_INTEGER TickCountBefore;
				QueryPerformanceCounter(&TickCountBefore);

				while(Win32GlobalRunning)
				{
					FILETIME DLLWriteTime = Win32FileLastWriteTime(FullPathToDll);
					if(CompareFileTime(&DLLWriteTime, &AppCode.DLLWriteTime))
					{
						Win32AppCodeUnload(&AppCode);
						AppCode = Win32AppCodeLoad(FullPathToDll, FullPathToDllTmp);
						AppCode.DLLWriteTime = DLLWriteTime;
					}

					app_keyboard_controller *OldKeyboardController = &OldInput->KeyboardController;
					app_keyboard_controller *NewKeyboardController = &NewInput->KeyboardController;
					app_keyboard_controller EmptyKeyboardController = {};
					*NewKeyboardController = EmptyKeyboardController;

					for(u32 ButtonIndex = 0; ButtonIndex < KEY_BUTTON_COUNT; ButtonIndex++)
					{
						NewKeyboardController->Buttons[ButtonIndex].EndedDown =
							OldKeyboardController->Buttons[ButtonIndex].EndedDown;
					}

					dMouseX = 0.0f;
					dMouseY = 0.0f;

					Win32ProcessPendingMessages(NewInput, NewKeyboardController);

					NewInput->dMouseX = dMouseX;
					NewInput->dMouseY = dMouseY;

					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0],
							GetKeyState(VK_LBUTTON) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1],
							GetKeyState(VK_MBUTTON) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2],
							GetKeyState(VK_RBUTTON) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[3],
							GetKeyState(VK_XBUTTON1) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[4],
							GetKeyState(VK_XBUTTON2) & (1 << 15));


					thread_context Thread = {};

					app_back_buffer AppBackBuffer = {};
					AppBackBuffer.Memory = Win32GlobalBackBuffer.Memory;
					AppBackBuffer.BytesPerPixel = Win32GlobalBackBuffer.BytesPerPixel;
					AppBackBuffer.Stride = Win32GlobalBackBuffer.Stride;
					AppBackBuffer.Width = Win32GlobalBackBuffer.Width;
					AppBackBuffer.Height = Win32GlobalBackBuffer.Height;

					AppCode.app_update_and_render(&Thread, &AppBackBuffer, NewInput, &AppMemory);

					StretchDIBits(DeviceContext,
							0, 0, Win32GlobalBackBuffer.Width, Win32GlobalBackBuffer.Height,
							0, 0, Win32GlobalBackBuffer.Width, Win32GlobalBackBuffer.Height,
							Win32GlobalBackBuffer.Memory,
							&Win32GlobalBackBuffer.Info,
							DIB_RGB_COLORS,
							SRCCOPY);

					LARGE_INTEGER TickCountAfter;
					QueryPerformanceCounter(&TickCountAfter);

					f32 TickCountElapsed = (f32)(TickCountAfter.QuadPart - TickCountBefore.QuadPart);
					dtForFrame = TickCountElapsed * TimeForEachTick;
					TickCountBefore = TickCountAfter;

					NewInput->dtForFrame = dtForFrame;
					app_input *TempInput = NewInput;
					NewInput = OldInput;
					OldInput = TempInput;

				}
			}
		}
	}
	return(0);
}
