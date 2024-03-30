#if !defined(SOFTWARE_RENDERER_H)

#include "software_renderer_platform.h"

struct memory_arena
{
	u8 *Base;
	memory_index Size;
	memory_index Used;
};

internal void
ArenaInitialize(memory_arena *Arena, memory_index Size, u8 *Base)
{
	Arena->Base = Base;
	Arena->Size = Size;
	Arena->Used = 0;
}

#define PushStruct(Arena, Type) (Type *)PushSize_(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, (Count) * sizeof(Type))

void *
PushSize_(memory_arena *Arena, memory_index Size)
{
	ASSERT((Arena->Used + Size) <= Arena->Size);

	void *Result = Arena->Base + Arena->Used;
	Arena->Used += Size;

	return(Result);
}

#include "software_renderer_math.h"
#include "software_renderer_intrinsics.h"

// TODO(Justin): Structs and functions dealing with geometric Dss need ot go in
// a geometry file


struct interval
{
	f32 Min;
	f32 Max;
};

struct aabb
{
	interval X;
	interval Y;
	interval Z;
};

struct rectangle
{
	v4f Min;
	v4f Max;
};

struct circle
{
	v4f Center;
	f32 Radius;
};

struct triangle
{
	v4f Pos;
	v3f Color;
	v4f Vertices[3];
};

struct quad
{
	v4f Pos;
	v4f Vertices[4];
};


struct edge
{
	f32 X;
	f32 XStep;
	s32 YStart;
	s32 YEnd;
};


// TODO(Justin): FIgure out what to store.
struct camera
{
	v3f Pos;
	v3f Direction;
	v3f Up;

	f32 Yaw;
	f32 Pitch;
};

struct app_state
{
	camera Camera;
	u32 CameraIndex;

	mat4 MapToCamera;
	mat4 MapToPersp;
	mat4 MapToScreenSpace;

	triangle Triangle;
	quad Quad;

	s32 MouseX;
	s32 MouseY;

	f32 Time;
};

#define SOFTWARE_RENDERER_H
#endif
