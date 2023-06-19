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

typedef struct
{
	v3f Pos;
	v3f Direction;
	v3f Up;
} camera;

typedef struct
{
	camera Cameras[3];
	u32 CameraIndex;

	m4x4 MapToCamera;
	m4x4 MapToPersp;
	m4x4 MapToScreenSpace;

	triangle Triangle;

} app_state;

#define SOFTWARE_RENDERER_H
#endif
