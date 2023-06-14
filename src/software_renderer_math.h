#if !defined(SOFTWARE_RENDERER_MATH_H)

#define SQUARE(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))

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

//
// NOTE(Justin): v2f operations
//

inline v2f
operator *(f32 c, v2f A)
{
	v2f Result;
	Result.x = c * A.x;
	Result.y = c * A.y;
	return(Result);
}

inline v2f
operator *(v2f A, f32 c)
{
	v2f Result = c * A;
	return(Result);
}


inline v2f &
operator *=(v2f &A, f32 c)
{
	A = c * A;
	return(A);
}

inline v2f
operator ~(v2f A)
{
	v2f Result = -1.0f * A;
	return(Result);
}

inline v2f
operator +(v2f A, v2f B)
{
	v2f Result;
	
	Result.x = A.x + B.x;
	Result.y = A.y + B.y;

	return(Result);
}

inline v2f &
operator +=(v2f &A, v2f B)
{
	A = A + B;
	return(A);
}

inline v2f
operator -(v2f A, v2f B)
{
	v2f Result;
	
	Result.x = A.x - B.x;
	Result.y = A.y - B.y;

	return(Result);
}

inline f32
v2f_innerf(v2f A, v2f B)
{
	f32 result = A.x * B.x + A.y * B.y;
	return(result);

}

inline f32
v2f_length_squaredf(v2f A)
{
	f32 result = v2f_innerf(A, A);
	return(result);
}

// NOTE(Justin): When should these functions return a v2f?
// WHen should these functions modify an arguement and return void?

inline v2f
v2f_normalizef(v2f A)
{
	v2f Result = A;

	f32 c = 1.0f / sqrt(v2f_innerf(A, A));
	Result *= c;

	return(Result);
}
inline v2f
v2f_crossf(v2f A, v2f B)
{
	v2f Result;

	Result.x = A.y * B.x - A.x * B.y;
	Result.y = A.x * B.y - A.y * B.x;

	return(Result);

	f32 c = 1.0f / sqrt(v2f_innerf(A, A));
	Result *= c;

	return(Result);
}

//
// NOTE(Justin): v3f operations
//

inline v3f
v3f_create_from_scalars(f32 x, f32 y, f32 z)
{
	v3f Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;

	return(Result);
}

inline v3f
v3f_create_from_v2f(v2f A, f32 z)

{
	v3f Result;

	Result.xy = A;
	Result.z = z;

	return(Result);
}

inline f32
v3f_innerf(v3f A, v3f B)
{
	f32 result = 0.0f;

	result = A.x * B.x + A.y * B.y + A.z * B.z;

	return(result);
}

inline f32
v3f_length_squared(v3f A)
{
	f32 result = v3f_innerf(A, A);

	return(result);
}



inline v3f
v3f_cross(v3f A, v3f B)
{
	v3f Result;

	Result.x = A.y * B.z - A.z * B.y;
	Result.y = A.z * B.x - A.x * B.z;
	Result.z = A.x * B.y - A.y * B.x;

	return(Result);
}

inline v3f
operator *(f32 c, v3f A)
{
	v3f Result;
	Result.x = c * A.x;
	Result.y = c * A.y;
	Result.z = c * A.z;

	return(Result);
}

inline v3f
operator *(v3f A, f32 c)
{
	v3f Result = c * A;

	return(Result);
}

inline v3f
operator ~(v3f A)
{
	v3f Result =  -1.0f * A;

	return(Result);
}

inline v3f
operator +(v3f A, v3f B)
{
	v3f Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return(Result);
}

inline v3f &
operator +=(v3f &A, v3f B)
{
	A = A + B;
	return(A);
}

inline v3f
operator -(v3f A, v3f B)
{
	v3f Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return(Result);
}

inline v3f
v3f_normalize(v3f A)
{
	v3f Result = A;

	f32 c = sqrt(v3f_innerf(A, A));
	c = 1.0f / c;

	Result = c * A;

	return(Result);
}


//
// NOTE(Justin): v4f operations
//

inline v4f
v4f_create_from_scalars(f32 x, f32 y, f32 z, f32 w)
{
	v4f Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;
	Result.w = w;

	return(Result);
}

inline v4f
v4f_create_from_v3f(v3f A, f32 w)
{
	v4f Result;

	Result.xyz = A;
	Result.w = w;

	return(Result);
}



inline v4f
operator +(v4f A, v4f B)
{
	v4f Result;
	Result.xyz = A.xyz + B.xyz;
	return(Result);
}

inline v4f
operator -(v4f A, v4f B)
{
	v4f Result;
	Result.xyz = A.xyz - B.xyz;
	return(Result);
}

inline v4f
operator *(f32 c, v4f A)
{
	v4f Result;
	Result.xyz = c * A.xyz;
	return(Result);
}

//
// NOTE(Justin): m2x2 operations
//

internal m2x2
operator *(m2x2 A, m2x2 B)
{
	// NOTE(Justin): Instructional, NOT optimized.
	m2x2 R = {};

	// For each row of A
	for (int r = 0; r <= 1; r++) {

		// For each col of B
		for (int c = 0; c <= 1; c++) {

			// Mul col of A by row of B (dot product)
			for (int i = 0; i <= 1; i++) {
				R.e[r][c] += A.e[r][i] * B.e[i][c];
			}
		}
	}
	return(R);
}

internal v2f
transform(m4x4 A, v2f V)
{
	v2f R;

	// NOTE(Justin): Instructional, NOT optimized.
	R.x = A.e[0][0] * V.x + A.e[0][1] * V.y;
	R.y = A.e[1][0] * V.x + A.e[1][1] * V.y;

	return(R);
}

inline v2f
operator *(m4x4 A, v2f V)
{
	v2f R = transform(A, V);//v4f_create_from_v3f(V, 1.0f)).xyz;
	return(R);
}

internal f32
m2x2_det(m2x2 A)
{
	f32 Result = 0;
	Result = A.e[0][0] * A.e[1][1] - A.e[0][1] * A.e[1][0];
	return(Result);
}

internal m2x2
m2x2_adjoint_create(m2x2 A)
{
	/*
	 * |a b|
	 * |c d|
	 *
	 * |d -b|
	 * |-c a|
	 *
	 * */

	m2x2 Result;
	Result.e[0][0] = A.e[1][1];
	Result.e[1][1] = A.e[0][0];
	Result.e[0][1] = -A.e[0][1];
	Result.e[1][0] = - A.e[1][0];

	return(Result);
}
//
// NOTE(Justin): m4x4 operations
//

internal m4x4
operator *(m4x4 A, m4x4 B)
{
	// NOTE(Justin): Instructional, NOT optimized.
	m4x4 R = {};

	// For each row of A
	for (int r = 0; r <= 3; r++) {

		// For each col of B
		for (int c = 0; c <= 3; c++) {

			// Mul col of A by row of B (dot product)
			for (int i = 0; i <= 3; i++) {
				R.e[r][c] += A.e[r][i] * B.e[i][c];
			}
		}
	}
	return(R);
}

internal v4f
transform(m4x4 A, v4f V)
{
	v4f R;

	// NOTE(Justin): Instructional, NOT optimized.
	R.x = A.e[0][0] * V.x + A.e[0][1] * V.y + A.e[0][2] * V.z + A.e[0][3] * V.w;
	R.y = A.e[1][0] * V.x + A.e[1][1] * V.y + A.e[1][2] * V.z + A.e[1][3] * V.w;
	R.z = A.e[2][0] * V.x + A.e[2][1] * V.y + A.e[2][2] * V.z + A.e[2][3] * V.w;
	R.w = A.e[3][0] * V.x + A.e[3][1] * V.y + A.e[3][2] * V.z + A.e[3][3] * V.w;

	return(R);
}

inline v3f
operator *(m4x4 A, v3f V)
{
	v3f R = transform(A, v4f_create_from_v3f(V, 1.0f)).xyz;
	return(R);
}

inline v4f
operator *(m4x4 A, v4f V)
{
	v4f R = transform(A, V);
	return(R);
}

inline m4x4
m4x4_identity_create(void)
{
	m4x4 R =
	{
		{{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline m4x4
m4x4_translation_create(v3f V)
{
	m4x4 R =
	{
		{{1, 0, 0, V.x},
		{0, 1, 0, V.y},
		{0, 0, 1, V.z},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline m4x4
m4x4_scale_create(v3f V)
{
	m4x4 R =
	{
		{{V.x, 0, 0, 0},
		{0, V.y, 0, 0},
		{0, 0, V.z, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline m4x4
m4x4_rotation_y_create(f32 angle)
{
	m4x4 R =
	{
		{{cos(angle), 0 , sin(angle), 0},
		{0, 1, 0, 0},
		{-1.0f * sin(angle), 0, cos(angle), 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline m4x4
m4x4_rotation_z_create(f32 angle)
{
	m4x4 R =
	{
		{{cos(angle), -1.0f * sin(angle), 0, 0},
		{sin(angle), cos(angle), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline m4x4
m4x4_rotation_x_create(f32 angle)
{
	m4x4 R =
	{
		{{1, 0, 0, 0},
		{0, cos(angle), sin(angle), 0},
		{0, -1.0f * sin(angle), cos(angle), 0},
		{0, 0, 0, 1}},
	};
	return(R);
}


inline m4x4
m4x4_perspective_projection_create(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
	f32 d = ((f + n) / (f - n));
	f32 e = (2.0f * f * n) / (f - n);

	m4x4 R =
	{
		{{2.0f * n / (r - l), 0, (r + l) / (r - l), 0},
		{0, 2.0f * n / (t - b), (t + b) / (t - b), 0},
		{0, 0, -d, -e},
		{0, 0, -1, 0}},
	};
	return(R);
}

#if 0
inline m4x4
m4x4_perspective_projection_create(f32 aspect_width_by_height, f32 focal_length, f32 plane_near, f32 plane_far)
{
	// NOTE(Justin): 
	//	n - f distance between near and far planes

	f32 a = 1.0f;
	f32 b = aspect_width_by_height;
	f32 c = focal_length;

	f32 n = plane_near;
	f32 f = plane_far;

	f32 d = (n + f) / (n - f);
	f32 e = (2.0f * f * n) / (n - f);

	m4x4 R =
	{
		{{(2.0f * n/ (r - l)), 0, 0, (-1.0f * (r + l) / (r - l))},
		{0, (2.0f / (t - b)), 0, (-1.0f * (t + b) / (t - b))},
		{0, 0, (2.0f / (n - f)), (-1.0f * (n + f) / (n - f))},
		{0, 0, 0, 1}},
	};
	return(R);
}
#endif

inline m4x4 
m4x4_rectangle_transformation(rectangle R1, rectangle R2)
{
	m4x4 R;

	v3f Shift = {-1.0f * R1.Min.x, -1.0f * R1.Min.y, 1.0f};

	f32 Scale_x = ((R2.Max.x - R2.Min.x) / (R1.Max.x - R1.Min.x));
	f32 Scale_y = ((R2.Max.y - R2.Min.y) / (R1.Max.y - R1.Min.y));
	v3f Scale = {Scale_x, Scale_y, 1.0f};

	m4x4 MTranslateToOrigin = m4x4_translation_create(Shift);
	m4x4 MScale = m4x4_scale_create(Scale);


	Shift = {R2.Min.x, R2.Min.y, 1.0f};

	m4x4 MTranslateToPosition = m4x4_translation_create(Shift);

	R = MTranslateToPosition * MScale * MTranslateToOrigin;

	return(R);
}

internal m4x4
m4x4_camera_map_create(v3f CameraPos, v3f CameraDirection, v3f CameraUp)
{
	m4x4 R;

	// The viewing transformations and definitions produce a -1 so that the
	// vector 0 0 1 is looking down the z axis
	//v3f W = -1.0f * v3f_normalize(CameraDirection);
	v3f W = v3f_normalize(CameraDirection);

	v3f U = v3f_cross(CameraUp, W);
	U = v3f_normalize(U);

	v3f V = v3f_cross(W, U);

	m4x4 MapToNewOrigin =
	{
		{{1, 0, 0, -CameraPos.x},
		{0, 1, 0, -CameraPos.y},
		{0, 0, 1, -CameraPos.z},
		{0, 0, 0, 1}},
	};

	// 
	m4x4 M =
	{
		{{U.x, V.x, W.x, 0},
		{U.y, V.y, W.y, 0},
		{U.z, V.z, W.z, 0},
		{0, 0, 0, 1}},
	};

	R = M * MapToNewOrigin;

	return(R);
}



internal m4x4
m4x4_screen_space_map_create(int width, int height)
{
	f32 nx = (f32)width;
	f32 ny = (f32)height;

	m4x4 R
	{
		{{nx / 2.0f, 0, 0, (nx - 1) / 2.0f},
		{0, ny / 2.0f, 0, (ny - 1) / 2.0f},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};


#if 0
	//NOTE(Justin): Do we need to subtract 1 from the shift then  / 2?
	// Asking question because read this matrix from book and they subtract 1
	// from the shift
	v3f x_scale_to_screen = v3f_create_from_scalars((f32)width / 2.0f, 1.0f, 1.0f);
	v3f x_shift_to_screen = v3f_create_from_scalars(-1.0f * (f32)(width - 1) / 2.0f, 0.0f, 0.0f);

	// WARNING(Justin): The order here matters. If we first shift then scale,
	// this produces an undesired result. Since matrix multiplication is from
	// right to left, the rightmost matrix must be the scaling transformation.
	// Then we apply the shifting transformation.

	m4x4 Scale_x = m4x4_scale_create(x_scale_to_screen);
	m4x4 Shift_x = m4x4_translation_create(x_shift_to_screen);
	m4x4 Map_x = Shift_x * Scale_x;

	v3f y_scale_to_screen = v3f_create_from_scalars(1.0f, (f32)height / 2.0f, 1.0f);
	v3f y_shift_to_screen = v3f_create_from_scalars(0.0f, -1.0f * (f32)(height - 1) / 2.0f, 0.0f);

	m4x4 Scale_y = m4x4_scale_create(y_scale_to_screen);
	m4x4 Shift_y = m4x4_translation_create(y_shift_to_screen);
	m4x4 Map_y = Shift_y * Scale_y;


	R = Map_y * Map_x;
#endif

	return(R);
}

internal m4x4
m4x4_orthographic_projection_create(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
	m4x4 R =
	{
		{{(2.0f / (r - l)), 0, 0, (-1.0f * (r + l) / (r - l))},
		{0, (2.0f / (t - b)), 0, (-1.0f * (t + b) / (t - b))},
		{0, 0, (2.0f / (f - n)), (-1.0f * (f + n) / (f - n))},
		{0, 0, 0, 1}},
	};
	return(R);
}


#if 0
internal m4x4
m4x4_translation_to_origin_create(basis_orthonormal B)
{
	m4x4 R =
	{
		{{1, 0, 0, -B.E1.x},
		{0, 1, 0, -B.E2.y},
		{0, 0, 1, -B.E3.z},
		{0, 0, 0, 1}},
	};
	return(R);
}
#endif
#define SOFTWARE_RENDERER_MATH_H
#endif
