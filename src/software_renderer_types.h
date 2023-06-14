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
#define ABS(x) ((x < 0) ? -(x): (x))

#define PI32 3.141592653589f
#define RAD32 (PI32 / 180.0f)

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

typedef struct
{
	void *memory;
	int height;
} scan_buffer;

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
} button_state;

typedef struct
{
	// NOTE(Justin): Should the just be a v2f?
	button_state Buttons[BUTTON_COUNT];
} input;

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

union v2i
{
	struct
	{
		s32 x, y;
	};
	struct
	{
		s32 u, v;
	};
	s32 e[2];
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



struct m2x2
{
	// NOTE(Justin): Stored in ROW MAJOR order i.e. e[ROW][COL]
	f32 e[2][2];
};

struct m4x4
{
	// NOTE(Justin): Stored in ROW MAJOR order i.e. e[ROW][COL]
	f32 e[4][4];
};

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
	v4f E1;
	v4f E2;
	v4f E3;
} basis;

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

#define SOFTWARE_RENDERER_TYPES_H
#endif
