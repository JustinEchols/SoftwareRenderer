#if !defined(SOFTWARE_RENDERER_INTRINSICS_H)
#include <math.h>

inline u32
U64TruncateToU32(u64 X)
{
	ASSERT(X <= 0xFFFFFFFF)
	u32 Result = (u32)X;

	return(Result);
}

inline s32
F32RoundToS32(f32 X)
{
	s32 Result = (s32)(X + 0.5f);

	return(Result);
}

inline s32
F32TruncateToS32(f32 X)
{
	s32 Result = (s32)X;
	return(Result);
}

inline u32
F32RoundToU32(f32 X)
{
	u32 Result = (u32)(X + 0.5f);
	return(Result);
}

inline u32
F32TruncateToU32(f32 X)
{
	u32 Result = (u32)X;
	return(Result);
}

inline s32
F32FloorToS32(f32 X)
{
	s32 Result = (s32)floorf(X);
	return(Result);
}

inline f32
Sqrt(f32 X)
{
	f32 Result = sqrtf(X);
	return(Result);
}

inline f32
Sin(f32 Angle)
{
	f32 Result = sinf(Angle);
	return(Result);
}

inline f32
Cos(f32 Angle)
{
	f32 Result = cosf(Angle);
	return(Result);
}

inline f32
Tan(f32 Angle)
{
	f32 Result = tanf(Angle);
	return(Result);
}

inline f32
Min(f32 X, f32 Y)
{
	f32 Result = fminf(X, Y);
	return(Result);
}

inline f32
Max(f32 X, f32 Y)
{
	f32 Result = fmaxf(X, Y);
	return(Result);
}

#define SOFTWARE_RENDERER_INTRINSICS_H
#endif

