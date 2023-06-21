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

#if APP_SLOW
#define ASSERT(expression) if (!(expression)){*(int *)0 = 0;}
#else
#define ASSERT(expression)
#endif

inline u32
u64_truncate_to_u32(u64 x)
{
	ASSERT(x <= 0xFFFFFFFF)
	u32 Result = (u32)x;
	return(Result);
}
//
// NOTE(Justin): Services that the application provides to the windows platform
// layer
//

#if APP_INTERNAL
typedef struct 
{
	u32 size;
	void *memory;
} debug_file_read_result;


#define DEBUG_PLATFORM_FILE_FREE(name) void name(void *memory)
typedef DEBUG_PLATFORM_FILE_FREE(debug_platform_file_free_func);

#define DEBUG_PLATFORM_FILE_READ_ENTIRE(name) debug_file_read_result name(char * file_name)
typedef DEBUG_PLATFORM_FILE_READ_ENTIRE(debug_platform_file_read_entire_func);

#define DEBUG_PLATFORM_FILE_WRITE_ENTIRE(name) b32 name(char *file_name, u32 size, void *memory)
typedef DEBUG_PLATFORM_FILE_WRITE_ENTIRE(debug_platform_file_write_entire_func);



#endif

typedef struct
{
	void *memory;
	int bytes_per_pixel;
	int stride;
	int width;
	int height;

} app_back_buffer;

enum
{
	BUTTON_W,
	BUTTON_S,
	BUTTON_A,
	BUTTON_D,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,

	BUTTON_COUNT,
};

typedef struct
{
	b32 changed;
	b32 is_down;

} app_button_state;

typedef struct
{
	f32 time_delta;
	app_button_state Buttons[BUTTON_COUNT];
} app_input;

typedef struct
{
	b32 is_initialized;
	void *permanent_storage;
	void *transient_storage;
	u64 permanent_storage_size;
	u64 transient_storage_size;

	debug_platform_file_read_entire_func	*debug_platform_file_read_entire;
	debug_platform_file_write_entire_func	*debug_platform_file_write_entire;
	debug_platform_file_free_func			*debug_platform_file_free;

} app_memory;



#define SFOTWARE_RENDERER_PLATFORM_H
#endif

