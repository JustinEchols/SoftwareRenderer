#if !defined(SOFTWARE_RENDERER_H)

#include "software_renderer_platform.h"
#include "software_renderer_math.h"
#include "software_renderer_intrinsics.h"

struct rectangle
{
	v4f Min;
	v4f Max;
};

struct circle
{
	v4f Center;
	f32 radius;
};


struct triangle
{
	v3f Color;
	v4f Vertices[3];
};

struct square
{
	triangle Triangles[2];
};

struct cube
{
	v4f Center;
	v2f Dim;
};

struct edge
{
	f32 x;
	f32 x_step;
	s32 y_start;
	s32 y_end;
};

#define APP_UPDATE_AND_RENDER(name) void name(app_back_buffer *AppBackBuffer, app_input *AppInput, app_memory *AppMemory)
typedef APP_UPDATE_AND_RENDER(app_update_and_render_func);
APP_UPDATE_AND_RENDER(app_update_and_render_stub)
{
}

struct camera
{
	v3f Pos;
	v3f Direction;
	v3f Up;
};

struct loaded_bitmap
{
	void *memory;
	s32 width;           
	s32 height;          

};

struct app_state
{
	camera Cameras[3];
	u32 CameraIndex;

	m4x4 MapToCamera;
	m4x4 MapToPersp;
	m4x4 MapToScreenSpace;

	triangle Triangle;

	loaded_bitmap Bitmap;
};

#define SOFTWARE_RENDERER_H
#endif
