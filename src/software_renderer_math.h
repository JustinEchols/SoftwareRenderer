#if !defined(SOFTWARE_RENDERER_MATH_H)

#define SQUARE(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))

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
v4f_create_from_v3f(v3f v, f32 w)
{
	v4f Result;

	Result.xyz = v;
	Result.w = w;

	return(Result);
}

//
// NOTE(Justin): m4x4 operations(matrix and vector)
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
transform(m4x4 A, v4f v)
{
	v4f R;

	// NOTE(Justin): Instructional, NOT optimized.
	R.x = A.e[0][0] * v.x + A.e[0][1] * v.y + A.e[0][2] * v.z + A.e[0][3] * v.w;
	R.y = A.e[1][0] * v.x + A.e[1][1] * v.y + A.e[1][2] * v.z + A.e[1][3] * v.w;
	R.z = A.e[2][0] * v.x + A.e[2][1] * v.y + A.e[2][2] * v.z + A.e[2][3] * v.w;
	R.w = A.e[3][0] * v.x + A.e[3][1] * v.y + A.e[3][2] * v.z + A.e[3][3] * v.w;

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
identity_create(void)
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

#define SOFTWARE_RENDERER_MATH_H
#endif
