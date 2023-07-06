#if !defined(SOFTWARE_RENDERER_H)

#include "software_renderer_platform.h"
#include "software_renderer_math.h"
#include "software_renderer_intrinsics.h"

typedef struct
{
	v4f Min;
	v4f Max;
} rectangle;

typedef struct
{
	v4f Center;
	f32 radius;
} circle;


typedef struct
{
	v3f Color;
	v4f Vertices[3];
} triangle;

typedef struct
{
	triangle Triangles[2];
} square;

typedef struct
{
	v4f Center;
	v2f Dim;
} cube;

typedef struct
{
	f32 x;
	f32 x_step;
	s32 y_start;
	s32 y_end;
} edge;

#define APP_UPDATE_AND_RENDER(name) void name(app_back_buffer *AppBackBuffer, app_input *AppInput, app_memory *AppMemory)
typedef APP_UPDATE_AND_RENDER(app_update_and_render_func);
APP_UPDATE_AND_RENDER(app_update_and_render_stub)
{
}

typedef struct
{
	v3f Pos;
	v3f Direction;
	v3f Up;
} camera;

typedef struct
{
	void *memory;
	s32 width;           
	s32 height;          

} loaded_bitmap;

typedef struct
{
	camera Cameras[3];
	u32 CameraIndex;

	m4x4 MapToCamera;
	m4x4 MapToPersp;
	m4x4 MapToScreenSpace;

	triangle Triangle;

	loaded_bitmap Bitmap;
} app_state;

#define SOFTWARE_RENDERER_H
#endif
