
//
// NOTE(Justin): Services that the application provides to the windows platform
// layer
//

#if !defined(SFOTWARE_RENDERER_PLATFORM_H)

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>


//
// NOTE(Justin): Fundamental data types.
//

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t memory_index;

typedef float f32;
typedef double f64;

//
// NOTE(Justin): Macros.
//

#define global_variable static
#define local_persist static
#define internal static

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))
#define ABS(x) ((x < 0) ? -(x): (x))

#define KILOBYTES(count) (1024 * (count))
#define MEGABYTES(count) (1024 * KILOBYTES(count))
#define GIGABYTES(count) (1024 * MEGABYTES(count))
#define TERABYTES(count) (1024 * GIGABYTES(count))

#define PI32 3.141592653589f
#define RAD32 (PI32 / 180.0f)
#define DegreeToRad(Theta) ((Theta) * RAD32)

#if APP_SLOW
#define ASSERT(expression) if (!(expression)){*(int *)0 = 0;}
#else
#define ASSERT(expression)
#endif

typedef struct
{
	int PlaceHolder;
} thread_context;

#if APP_INTERNAL
typedef struct 
{
	u32 Size;
	void *Memory;
} debug_file_read_result;


#define DEBUG_PLATFORM_FILE_FREE(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FILE_FREE(debug_platform_file_free_func);

#define DEBUG_PLATFORM_FILE_READ_ENTIRE(name) debug_file_read_result name(thread_context *Thread, char *FileName)
typedef DEBUG_PLATFORM_FILE_READ_ENTIRE(debug_platform_file_read_entire_func);

#define DEBUG_PLATFORM_FILE_WRITE_ENTIRE(name) b32 name(thread_context *Thread, char *FileName, u32 Size, void *Memory)
typedef DEBUG_PLATFORM_FILE_WRITE_ENTIRE(debug_platform_file_write_entire_func);

#endif

typedef struct
{
	void *Memory;
	int BytesPerPixel;
	int Stride;
	int Width;
	int Height;

} app_back_buffer;

enum
{
	KEY_BUTTON_W,
	KEY_BUTTON_A,
	KEY_BUTTON_S,
	KEY_BUTTON_D,
	KEY_BUTTON_UP,
	KEY_BUTTON_DOWN,
	KEY_BUTTON_LEFT,
	KEY_BUTTON_RIGHT,
	KEY_BUTTON_1,
	KEY_BUTTON_2,
	KEY_BUTTON_3,

	KEY_BUTTON_COUNT,
};


typedef struct
{
	b32 EndedDown;
	u32 HalfTransitionCount;

} app_button_state;

typedef struct
{
	union
	{
		app_button_state Buttons[KEY_BUTTON_COUNT];
		struct
		{
			app_button_state W;
			app_button_state A;
			app_button_state S;
			app_button_state D;
			app_button_state Up;
			app_button_state Down;
			app_button_state Left;
			app_button_state Right;
			app_button_state One;
			app_button_state Two;
			app_button_state Three;
		};
	};
} app_keyboard_controller;

typedef struct
{
	app_button_state MouseButtons[5];
	s32 MouseX, MouseY, MouseZ;
	f32 dMouseX, dMouseY;

	f32 dtForFrame;
	app_keyboard_controller KeyboardController;
} app_input;

typedef struct
{
	b32 IsInitialized;
	void *PermanentStorage;
	void *TransientStorage;
	u64 PermanentStorageSize;
	u64 TransientStorageSize;

	debug_platform_file_read_entire_func	*debug_platform_file_read_entire;
	debug_platform_file_write_entire_func	*debug_platform_file_write_entire;
	debug_platform_file_free_func			*debug_platform_file_free;

} app_memory;

#define APP_UPDATE_AND_RENDER(name) void name(thread_context *Thread, app_back_buffer *AppBackBuffer, app_input *AppInput, app_memory *AppMemory)
typedef APP_UPDATE_AND_RENDER(app_update_and_render_func);
APP_UPDATE_AND_RENDER(app_update_and_render_stub)
{
}

#define SFOTWARE_RENDERER_PLATFORM_H
#endif

