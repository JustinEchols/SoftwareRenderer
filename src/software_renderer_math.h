#if !defined(SOFTWARE_RENDERER_MATH_H)
#include <math.h>

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

struct mat4
{
	// NOTE(Justin): Stored in ROW MAJOR order i.e. e[ROW][COL]
	f32 e[4][4];
};

//
// NOTE(Justin): Scalar operations
//

inline f32
Clamp(f32 A, f32 t, f32 B)
{
	f32 Result = t;

	if(Result < A)
	{
		Result = A;
	}

	if(Result > B)
	{
		Result = B;
	}

	return(Result);
}

inline f32
Clamp01(f32 X)
{
	f32 Result = Clamp(0.0f, X, 1.0f);

	return(Result);
}

//
// NOTE(Justin): v2 operations
//

inline v2i
V2I(s32 X, s32 Y)
{
	v2i Result;
	Result.x = X;
	Result.y = Y;
	return(Result);
}

inline v2i
operator +(v2i U, v2i V)
{
	v2i Result = {};
	Result.x = U.x + V.x;
	Result.y = U.y + V.y;

	return(Result);
}

inline v2i
operator *(f32 c, v2i V)
{
	v2i Result;
	Result.x = (s32)(c * (f32)V.x);
	Result.y = (s32)(c * (f32)V.y);

	return(Result);
}

inline v2f
V2F(f32 X, f32 Y)
{
	v2f Result;
	Result.x = X;
	Result.y = Y;

	return(Result);
}

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
Dot(v2f A, v2f B)
{
	f32 Result = A.x * B.x + A.y * B.y;
	return(Result);

}

inline f32
LengthSq(v2f A)
{
	f32 Result = Dot(A, A);
	return(Result);
}

inline v2f
Normalize(v2f A)
{
	v2f Result = A;

	f32 c = 1.0f / sqrt(Dot(A, A));
	Result *= c;

	return(Result);
}
inline v2f
Cross(v2f A, v2f B)
{
	v2f Result;

	Result.x = A.y * B.x - A.x * B.y;
	Result.y = A.x * B.y - A.y * B.x;

	return(Result);

	f32 c = 1.0f / sqrt(Dot(A, A));
	Result *= c;

	return(Result);
}

inline f32
Det(v2f A, v2f B)
{
	f32 Result = 0.0f;

	Result = A.x * B.y - A.y * B.x;

	return(Result);
}

//
// NOTE(Justin): v3f operations
//


inline v3f
V3F(f32 X, f32 Y, f32 Z)
{
	v3f Result;

	Result.x = X;
	Result.y = Y;
	Result.z = Z;

	return(Result);
}

inline v3f
V3F(f32 C)
{
	v3f Result;

	Result.x = C;
	Result.y = C;
	Result.z = C;

	return(Result);
}

inline v3f
V3fCreateFromV2f(v2f A, f32 Z)

{
	v3f Result;

	Result.xy = A;
	Result.z = Z;

	return(Result);
}

inline f32
Dot(v3f A, v3f B)
{
	f32 Result = 0.0f;

	Result = A.x * B.x + A.y * B.y + A.z * B.z;

	return(Result);
}

inline f32
LengthSq(v3f A)
{
	f32 Result = Dot(A, A);

	return(Result);
}

inline v3f
Cross(v3f A, v3f B)
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
Normalize(v3f A)
{
	v3f Result = A;

	f32 c = sqrt(Dot(A, A));
	ASSERT(c != 0);
	c = 1.0f / c;

	Result = c * A;

	return(Result);
}

inline v3f
XAxis()
{
	v3f Result = {1.0f, 0.0f, 0.0f};
	return(Result);
}

inline v3f
YAxis()
{
	v3f Result = {0.0f, 1.0f, 0.0f};
	return(Result);
}

inline v3f
ZAxis()
{
	v3f Result = {0.0f, 0.0f, 1.0f};
	return(Result);
}

//
// NOTE(Justin): v4f operations
//

inline v4f
V4F(f32 X, f32 Y, f32 Z, f32 W)
{
	v4f Result;

	Result.x = X;
	Result.y = Y;
	Result.z = Z;
	Result.w = W;

	return(Result);
}

inline v4f
v4f_create_from_v3f(v3f A, f32 W)
{
	v4f Result;

	Result.xyz = A;
	Result.w = W;

	return(Result);
}

inline v4f
operator +(v4f A, v4f B)
{
	v4f Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;
	Result.w = A.w + B.w;

	return(Result);
}

inline v4f &
operator +=(v4f &A, v4f B)
{
	A = A + B;
	return(A);
}

inline v4f
operator -(v4f A, v4f B)
{
	v4f Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;
	Result.w = A.w - B.w;

	return(Result);
}

inline v4f
operator *(f32 c, v4f A)
{
	v4f Result;

	Result.x = c * A.x;
	Result.y = c * A.y;
	Result.z = c * A.z;
	Result.w = c * A.w;

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
m2x2_transform(m2x2 A, v2f V)
{
	v2f R;

	// NOTE(Justin): Instructional, NOT optimized.
	R.x = A.e[0][0] * V.x + A.e[0][1] * V.y;
	R.y = A.e[1][0] * V.x + A.e[1][1] * V.y;

	return(R);
}

inline v2f
operator *(m2x2 A, v2f V)
{
	v2f R = m2x2_transform(A, V);
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
	m2x2 Result;
	Result.e[0][0] = A.e[1][1];
	Result.e[1][1] = A.e[0][0];
	Result.e[0][1] = -A.e[0][1];
	Result.e[1][0] = - A.e[1][0];

	return(Result);
}

//
// NOTE(Justin): mat4 operations
//

internal mat4
operator *(mat4 A, mat4 B)
{
	// NOTE(Justin): Instructional, NOT optimized.
	mat4 R = {};

	// For each row of A
	for (int r = 0; r <= 3; r++) {

		// For each col of B
		for (int c = 0; c <= 3; c++) {

			// Mul row of A by col of B (dot product)
			for (int i = 0; i <= 3; i++) {
				R.e[r][c] += A.e[r][i] * B.e[i][c];
			}
		}
	}
	return(R);
}

internal v4f
Mat4Transform(mat4 A, v4f V)
{
	v4f R;

	// NOTE(Justin): Instructional, NOT optimized.
	R.x = A.e[0][0] * V.x + A.e[0][1] * V.y + A.e[0][2] * V.z + A.e[0][3] * V.w;
	R.y = A.e[1][0] * V.x + A.e[1][1] * V.y + A.e[1][2] * V.z + A.e[1][3] * V.w;
	R.z = A.e[2][0] * V.x + A.e[2][1] * V.y + A.e[2][2] * V.z + A.e[2][3] * V.w;
	R.w = A.e[3][0] * V.x + A.e[3][1] * V.y + A.e[3][2] * V.z + A.e[3][3] * V.w;

	return(R);
}

inline mat4
Mat4(v3f E1, v3f E2, v3f E3)
{
	mat4 R;

	R =
	{
		{{E1.x, E2.x, E3.x, 0.0f},
		{E1.y, E2.y, E3.y, 0.0f},
		{E1.z, E2.z, E3.z, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}}
	};

	return(R);
}

inline v3f
operator *(mat4 A, v3f V)
{
	v3f R = Mat4Transform(A, v4f_create_from_v3f(V, 1.0f)).xyz;
	return(R);
}

inline v4f
operator *(mat4 A, v4f V)
{
	v4f R = Mat4Transform(A, V);
	return(R);
}

inline mat4
Mat4Identity(void)
{
	mat4 R =
	{
		{{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4Translation(v3f V)
{
	mat4 R =
	{
		{{1, 0, 0, V.x},
		{0, 1, 0, V.y},
		{0, 0, 1, V.z},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4Scale(v3f V)
{
	mat4 R =
	{
		{{V.x, 0, 0, 0},
		{0, V.y, 0, 0},
		{0, 0, V.z, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4XRotation(f32 angle)
{
	mat4 R =
	{
		{{1, 0, 0, 0},
		{0, cos(angle), -1.0f * sin(angle), 0},
		{0, sin(angle), cos(angle), 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4YRotation(f32 angle)
{
	mat4 R =
	{
		{{cos(angle), 0 , sin(angle), 0},
		{0, 1, 0, 0},
		{-1.0f * sin(angle), 0, cos(angle), 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4ZRotation(f32 angle)
{
	mat4 R =
	{
		{{cos(angle), -1.0f * sin(angle), 0, 0},
		{sin(angle), cos(angle), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4TransposeMat3(mat4 M)
{
	mat4 R = M;

	for(u32 j = 0; j < 3; ++j)
	{
		for(u32 i = 0; i < 3; ++i)
		{
			if((i != j) && (i < j))
			{
				f32 Temp = R.e[j][i];
				R.e[j][i] = R.e[i][j];
				R.e[i][j] = Temp;
			}
		}
	}

	return(R);
}

inline mat4
Mat4WorldSpaceMap(v3f V)
{
	mat4 R = Mat4Translation(V);
	return(R);
}

inline mat4
Mat4PerspectiveProjection(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
	f32 d = ((f + n) / (f - n));
	f32 e = (2.0f * f * n) / (f - n);

	mat4 R =
	{
		{{2.0f * n / (r - l), 0, (r + l) / (r - l), 0},
		{0, 2.0f * n / (t - b), (t + b) / (t - b), 0},
		{0, 0, -d, -e},
		{0, 0, -1, 0}},
	};
	return(R);
}

inline mat4
Mat4PerspectiveGL(f32 FOV, f32 AspectRatio, f32 ZNear, f32 ZFar)
{
	mat4 R = {};

	f32 HFOV = FOV / 2.0f;

	R.e[0][0] = 1.0f / (tanf(HFOV) * AspectRatio);
    R.e[1][1] = 1.0f / tanf(HFOV);
    R.e[2][3] = 1.0f;
    R.e[2][2] = (ZFar + ZNear) / (ZFar - ZNear);
    R.e[3][2] = -(2.0f *ZFar*ZNear) / (ZFar - ZNear);

	return(R);
}

internal mat4
Mat4CameraMap(v3f P, v3f Target)
{
	mat4 R;

	// NOTE(Justin): The 3 vectors constructed from P and Target are basis vectors that
	// are column vectors of the rotation matrix. The Mat4 function puts the
	// vectors in a mat4 as COLUMN vectors. I.e.
	//
	//	Mat4(X, Y, Z) = |X Y Z 0|
	//					|0 0 0 1|
	//
	// After constructing this matrix the final rotation is the inverse of this
	// matrix which is equivalent to its transpose.

	v3f CameraDirection = Normalize(P - Target);
	v3f CameraRight = Normalize(Cross(YAxis(), CameraDirection));
	v3f CameraUp = Normalize(Cross(CameraDirection, CameraRight));

	mat4 Rotate = Mat4TransposeMat3(Mat4(CameraRight, CameraUp, CameraDirection));
	//mat4 Rotate = Mat4(CameraRight, CameraUp, CameraDirection);

	mat4 Translate = Mat4Translation(-1.0f * P);

	R = Rotate * Translate;

	return(R);
}

// NOTE(Justin): Should the screen space map include the perspective divide
internal mat4
Mat4ScreenSpaceMap(int Width, int Height)
{
	mat4 R
	{
		{{Width / 2.0f, 0, 0, (Width - 1) / 2.0f},
		{0, Height / 2.0f, 0, (Height - 1) / 2.0f},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};

	return(R);
}

internal mat4
Mat4OrthographicProjection(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
	mat4 R =
	{
		{{(2.0f / (r - l)), 0, 0, (-1.0f * (r + l) / (r - l))},
		{0, (2.0f / (t - b)), 0, (-1.0f * (t + b) / (t - b))},
		{0, 0, (2.0f / (f - n)), (-1.0f * (f + n) / (f - n))},
		{0, 0, 0, 1}},
	};
	return(R);
}

#define SOFTWARE_RENDERER_MATH_H
#endif
