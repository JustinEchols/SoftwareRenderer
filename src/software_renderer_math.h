#if !defined(SOFTWARE_RENDERER_MATH_H)
#include "math.h"

#define SQUARE(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))

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

#if 0
inline m4x4
m4x4_transpose(m4x4 A)
{
	m4x4 R;

	f32 tmp = 0.0f;
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			tmp = A[i][j];
			A[i][j] = A[j][i];
			A[j][i] = tmp;
		}
	}
}
#endif

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
#endif

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


internal m4x4
m4x4_translation_to_origin_create(v3f V)
{
	m4x4 R =
	{
		{{1, 0, 0, -V.x},
		{0, 1, 0, -V.y},
		{0, 0, 1, -V.z},
		{0, 0, 0, 1}},
	};
	return(R);
}
#define SOFTWARE_RENDERER_MATH_H
#endif
