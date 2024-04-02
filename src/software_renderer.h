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

#include "software_renderer_intrinsics.h"
#include "software_renderer_math.h"


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





// TODO(Justin): FIgure out what to store.
struct camera
{
	v3f Pos;
	v3f Direction;
	v3f Up;

	f32 Yaw;
	f32 Pitch;
};

// NOTE(Justin): When loading a model the members of a mesh that need to have
// memory allocated to them are the members that are pointers. If we want to
// load any model and any mesh we cannot specify how many there are because we
// do not know ahead of time. Instead we can count how many there are when
// processing the file contents. Once we have a count then we can allocate an
// array. We would need to do this if we did not want to use a dynamic array
// that can change size.

struct mesh
{
	u32 *Indices;
	v3f *Vertices;
	v2f *TexCoords;
	v3f *Normals;

	// NOTE(Justin): A face is, in this context, is defined as 3-tuple in the
	// file as x/y/z. The format is {vertex position index} / {vertex texture
	// index} / {vertex normal index}.

	u32 *Faces;

	u32 VertexCount;
	u32 TexCoordCount;
	u32 NormalCount;
	u32 FaceCount;

	u32 FaceStride;
	u32 FaceIndexCount;
};

struct loaded_obj
{
	char *FileName;

	u8 *Memory;
	u32 Size;

	mesh Mesh;
};

struct app_state
{
	memory_arena WorldArena;

	loaded_obj Cube;
	loaded_obj Suzanne;

	mesh CubeMesh;
	mesh SuzanneMesh;

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
