#if !defined(SOFTWARE_RENDERER_INTRINSICS_H)

inline u32
U64TruncateToU32(u64 X)
{
	ASSERT(X <= 0xFFFFFFFF)
	u32 Result = (u32)X;

	return(Result);
}

internal s32
F32RoundToS32(f32 X)
{
	s32 Result = (s32)(X + 0.5f);

	return(Result);
}

internal s32
F32TruncateToS32(f32 X)
{
	s32 Result = (s32)X;
	return(Result);
}

internal u32
F32RoundToU32(f32 X)
{
	u32 Result = (u32)(X + 0.5f);
	return(Result);
}

internal u32
F32TruncateToU32(f32 X)
{
	u32 Result = (u32)X;
	return(Result);
}

inline f32
Sqrt(f32 X)
{
	f32 Result = sqrtf(X);
	return(Result);
}

#define SOFTWARE_RENDERER_INTRINSICS_H
#endif

