#if !defined(SOFTWARE_RENDERER_TYPES_H)

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <limits.h>
#include <float.h>

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

#define global_variable static
#define local_persist static
#define internal static

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))


//#define ASSERT(expression) if(!expression){*(int *)0 = 0;}



typedef struct
{
	BITMAPINFO Info;
	void *memory;
	int bytes_per_pixel;
	int stride;
	int width;
	int height;

} win32_back_buffer;

union v2f
{
	struct
	{
		f32 x, y;
	};
	struct
	{
		f32 u, v;
	};
	f32 e[2];
};

union v3f
{
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 u, v, __;
	};
	struct
	{
		f32 r, g, b;
	};
	struct
	{
		v2f xy;
		f32 ignored0_;
	};
	struct
	{
		f32 ignored1_;
		v2f yz;

	};
	struct
	{
		v2f uv;
		f32 ignored2_;
	};
	struct
	{
		f32 ignored3_;
		v2f v__;
	};
	f32 e[3];
};

union v4f
{
	struct
	{
		union
		{
			v3f xyz;
			struct
			{
				f32 x, y, z;
			};
		};
		f32 w;
	};
	struct
	{
		union
		{
			v3f rgb;
			struct
			{
				f32 r, g, b;
			};
		};
		f32 a;
	};
	struct
	{
		v2f xy;
		f32 ignored0_;
		f32 ignored1_;
	};
	struct
	{

		f32 ignored2_;
		v2f yz;
		f32 ignored3_;
	};
	struct
	{

		f32 ignored4_;
		f32 ignored5_;
		v2f zw;
	};
	f32 e[4];
};


struct m4x4
{
	// NOTE(Justin): Stored in ROW MAJOR order i.e. e[ROW][COL]
	f32 e[4][4];
};

typedef struct
{
	v2f Center;
	f32 radius;
} circle;

// NOTE(Justin): should a color be a property of a triangle that we can set/modify? 
typedef struct
{
	v3f Vertices[3];
} triangle;

typedef struct
{
	triangle Triangles[2];
} square;

#define SOFTWARE_RENDERER_TYPES_H
#endif
