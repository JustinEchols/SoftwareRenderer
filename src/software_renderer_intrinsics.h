#if !defined(SOFTWARE_RENDERER_INTRINSICS_H)

internal s32
round_f32_to_s32(f32 x)
{
	s32 result = (s32)(x + 0.5f);
	return(result);
}

internal s32
truncate_f32_to_s32(f32 x)
{
	s32 result = (s32)x;
	return(result);
}

internal u32
round_f32_to_u32(f32 x)
{
	u32 result = (u32)(x + 0.5f);
	return(result);
}

internal u32
truncate_f32_to_u32(f32 x)
{
	u32 result = (u32)x;
	return(result);
}

inline f32
sqauare_rootf(f32 x)
{
	f32 result = sqrtf(x);
	return(result);
}

#define SOFTWARE_RENDERER_INTRINSICS_H
#endif

