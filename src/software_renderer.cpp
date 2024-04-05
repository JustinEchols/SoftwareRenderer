#include "software_renderer.h"

internal v3f
ColorRandInit()
{
	v3f Result;
	Result.r = ((f32)rand() / (f32)RAND_MAX);
	Result.g = ((f32)rand() / (f32)RAND_MAX);
	Result.b = ((f32)rand() / (f32)RAND_MAX);

	return(Result);
}

internal v4f
V4fRandInit()
{
	v4f Result = {};

	Result.x = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
	Result.y = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
	Result.z = (((f32)rand() / (f32)RAND_MAX) - 2.0f);
	Result.w = 1;

	return(Result);
}

internal triangle
TriangleRandInit()
{
	triangle Result = {};

	Result.Pos = V4fRandInit();

	for(u32 i = 0; i < 4; i++)
	{
		Result.Vertices[i] = V4fRandInit();
	}

	Result.Color = ColorRandInit();

	return(Result);
}

internal triangle
TriangleInit(v4f Pos, v4f Vertices[3])
{
	triangle Result = {};

	return(Result);

}

internal u32
ColorConvertV3fToU32(v3f Color)
{
	u32 Result = 0;

	u32 red		= (u32)(255.0f * Color.r);
	u32 green	= (u32)(255.0f * Color.g);
	u32 blue	= (u32)(255.0f * Color.b);
	Result		= ((red << 16) | (green << 8) | (blue << 0));

	return(Result);
}

internal void
PixelSet(app_back_buffer *AppBackBuffer, v2f PixelPos, v3f Color)
{
	s32 X = F32RoundToS32(PixelPos.x);
	s32 Y = F32RoundToS32(PixelPos.y);

	if((X < 0) || (X >= AppBackBuffer->Width))
	{
		return;
	}
	if((Y < 0) || (Y >= AppBackBuffer->Height))
	{
		return;
	}

	u32 color = ColorConvertV3fToU32(Color);

	u8 *PixelRow = (u8 *)AppBackBuffer->Memory + AppBackBuffer->Stride * Y + AppBackBuffer->BytesPerPixel * X;
	u32 *Pixel = (u32 *)PixelRow;
	*Pixel = color;
}

internal void
LineDDADraw(app_back_buffer *AppBackBuffer, v2f P1, v2f P2, v3f Color)
{
	v2f Diff = P2 - P1;

	s32 dX = F32RoundToS32(Diff.x);
	s32 dY = F32RoundToS32(Diff.y);

	s32 PixelCount, PixelIndex;

	if(ABS(dX) > ABS(dY))
	{
		PixelCount = ABS(dX);
	}
	else
	{
		PixelCount = ABS(dY);
	}

	v2f Increment = {(f32)dX / (f32)PixelCount, (f32)dY / (f32)PixelCount};
	v2f PixelPos = P1;

	PixelSet(AppBackBuffer, PixelPos, Color);
	for(PixelIndex = 0; PixelIndex < PixelCount; PixelIndex++)
	{
		PixelPos += Increment;
		PixelSet(AppBackBuffer, PixelPos, Color);
	}
}

// NOTE(Justin): Unpack4x8?
internal v4f
UnpackU32ToV4F(u32 Packed)
{
	v4f Result =
	{
		(f32)((Packed >> 16) & 0xFF),
		(f32)((Packed >> 8) & 0xFF),
		(f32)((Packed >> 0) & 0xFF),
		(f32)((Packed >> 24) & 0xFF)
	};

	return(Result);
}

internal void
LineDDABilinearDraw(app_back_buffer *AppBackBuffer, v2f P1, v2f P2, v3f Color)
{
	v2f Diff = P2 - P1;

	s32 dX = F32RoundToS32(Diff.x);
	s32 dY = F32RoundToS32(Diff.y);

	s32 PixelCount, PixelIndex;

	if(ABS(dX) > ABS(dY))
	{
		PixelCount = ABS(dX);
	}
	else
	{
		PixelCount = ABS(dY);
	}

	v2f Increment = {(f32)dX / (f32)PixelCount, (f32)dY / (f32)PixelCount};
	v2f PixelPos = P1;

	PixelSet(AppBackBuffer, PixelPos, Color);
	for(PixelIndex = 0; PixelIndex < PixelCount; PixelIndex++)
	{
		PixelPos += Increment;
		s32 X = F32FloorToS32(PixelPos.x);
		s32 Y = F32FloorToS32(PixelPos.y);

		f32 tx = PixelPos.x - (f32)X;
		f32 ty = PixelPos.y - (f32)Y;

		u8 *PixelRow = (u8 *)AppBackBuffer->Memory + Y + X * sizeof(u32);

		u32 *PixelA = (u32 *)PixelRow;
		u32 *PixelB = (u32 *)(PixelRow + sizeof(u32));
		u32 *PixelC = (u32 *)(PixelRow + AppBackBuffer->Stride);
		u32 *PixelD = (u32 *)(PixelRow + AppBackBuffer->Stride + sizeof(u32));

		v4f A = UnpackU32ToV4F(*PixelA);
		v4f B = UnpackU32ToV4F(*PixelB);
		v4f C = UnpackU32ToV4F(*PixelC);
		v4f D = UnpackU32ToV4F(*PixelD);

		v4f Interpolant1 = Lerp(A, tx, B);
		v4f Interpolant2 = Lerp(C, tx, D);
		v4f Interpolant = Lerp(Interpolant1, ty, Interpolant2);

		v4f FinalColor = Hadamard(Interpolant, V4FCreateFromV3F(Color, 1.0f));

		//u32 R = F32RoundToU32(255.0f * C.r);
		//u32 G = F32RoundToU32(255.0f * C.g);
		//u32 B = F32RoundToU32(255.0f * C.b);
		//u32 A = F32RoundToU32(255.0f * C.a);

		PixelSet(AppBackBuffer, PixelPos, FinalColor.rgb);

	}
}

internal void
CircleDraw(app_back_buffer *AppBackBuffer, circle Circle, v3f Color)
{
#if 0
	v2f Radius = {Circle.Radius, Circle.Radius};
	v2f Min = Circle.Center.xy - Radius;
	v2f Max = Circle.Center.xy + Radius;
#endif

	u32 x_min = F32RoundToU32(Circle.Center.x - Circle.Radius);
	u32 x_max = F32RoundToU32(Circle.Center.x + Circle.Radius);

	u32 y_min = F32RoundToU32(Circle.Center.y - Circle.Radius);
	u32 y_max = F32RoundToU32(Circle.Center.y + Circle.Radius);

	f32 distance_squared = 0.0f;
	f32 Radius_sqaured = SQUARE(Circle.Radius);

	v2f Test;
	for(u32 y = y_min; y < y_max; y++)
	{
		for(u32 x = x_min; x < x_max; x++)
		{
			v2f PixelCurrent = {(f32)x, (f32)y};
			Test = Circle.Center.xy - PixelCurrent;

			distance_squared = Dot(Test, Test);
			if(distance_squared <= Radius_sqaured)
			{
				PixelSet(AppBackBuffer, Test, Color);
			}
		}
	}
}



internal void
RectangleDraw(app_back_buffer *AppBackBuffer, v2f Min, v2f Max, v3f Color)
{
	s32 XMin = F32RoundToU32(Min.x);
	s32 XMax = F32RoundToU32(Max.x);

	s32 YMin = F32RoundToU32(Min.y);
	s32 YMax = F32RoundToU32(Max.y);

	if(XMin < 0)
	{
		XMin = 0;
	}
	if(XMax > AppBackBuffer->Width)
	{
		XMax = AppBackBuffer->Width;
	}
	if(YMin < 0)
	{
		YMin = 0;
	}
	if(YMax > AppBackBuffer->Height)
	{
		YMax = AppBackBuffer->Height;
	}

	u32 color = ColorConvertV3fToU32(Color);

	u8 *PixelRow = (u8 *)AppBackBuffer->Memory + AppBackBuffer->Stride * YMin + AppBackBuffer->BytesPerPixel * XMin;
	for(s32 Y = YMin; Y < YMax; ++Y)
	{
		u32 *Pixel = (u32 *)PixelRow;
		for(s32 X = XMin; X < XMax; ++X)
		{
			*Pixel++ = color;
		}
		PixelRow += AppBackBuffer->Stride;
	}
}

internal void
AxisDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, v4f P)
{
	f32 Offset = 1.0f;

	v4f dX = V4F(Offset, 0.0f, 0.0f, 0.0f);
	v4f dY = V4F(0.0f, Offset, 0.0f, 0.0f);
	v4f dZ = V4F(0.0f, 0.0f, -Offset, 0.0f);

	v4f PX = P + dX;
	v4f PY = P + dY;
	v4f PZ = P + dZ;

	P = Mat4MVP * P;
	PX = Mat4MVP * PX;
	PY = Mat4MVP * PY;
	PZ = Mat4MVP * PZ;

	P = (1.0f / P.w) * P;
	PX = (1.0f / PX.w) * PX;
	PY = (1.0f / PY.w) * PY;
	PZ = (1.0f / PZ.w) * PZ;

	P = Mat4ScreenSpace * P;
	PX = Mat4ScreenSpace * PX;
	PY = Mat4ScreenSpace * PY;
	PZ = Mat4ScreenSpace * PZ;

	// NOTE(Justin): x axis
	v3f Color = {1.0f, 0.0f, 0.0f};
	LineDDADraw(AppBackBuffer, P.xy, PX.xy, Color);

	// NOTE(Justin): y axis
	Color = {0.0f, 1.0f, 0.0f};
	LineDDADraw(AppBackBuffer, P.xy, PY.xy, Color);

	// NOTE(Justin): z axis
	Color = {0.0f, 0.0f, 1.0f};
	LineDDADraw(AppBackBuffer, P.xy, PZ.xy, Color);
}


internal v3f
Barycentric(v3f X, v3f Y, v3f Z, v3f P)
{
	//
	// NOTE(Justin): The computation of finding the cood. is based on the area
	// interpretation of Barycentric coordinates.
	//
	
	v3f Result = {};
	v3f E1 = Z - X;
	v3f E2 = Y - X;
	v3f F = P - X;

	v3f Beta = {};
	v3f Gamma = {};

	Beta = (Dot(E2, E2) * E1 - Dot(E1, E2) * E2); 
	f32 CBeta = 1.0f / (Dot(E1, E1) * Dot(E2, E2) - SQUARE(Dot(E1, E2)));
	Beta = CBeta * Beta; 

	Gamma = (Dot(E1, E1) * E2 - Dot(E1, E2) * E1); 
	f32 CGamma = 1.0f / (Dot(E1, E1) * Dot(E2, E2) - SQUARE(Dot(E1, E2)));
	Gamma = CGamma * Gamma;

	Result.x = Dot(Beta, F);
	Result.y = Dot(Gamma, F);
	Result.z = 1 - Result.x - Result.y;

	return(Result);
}

internal v3f
BarycentricV2(v2f V0, v2f V1, v2f V2, v2f P)
{
	v3f Result = {};

	f32 SignedDoubleArea = Det(V1 - V0, V2 - V0);

	if(SignedDoubleArea != 0.0f)
	{
		f32 E01 = Det((V1 - V0), (P - V0));
		f32 E12 = Det((V2 - V1), (P - V1));
		f32 E20 = Det((V0 - V2), (P - V2));

		f32 C1 = E01 / SignedDoubleArea;
		f32 C2 = E12 / SignedDoubleArea;
		f32 C3 = E20 / SignedDoubleArea;

		Result = V3F(C1, C2, C3);
	}

	return(Result);
}

internal void
TriangleDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, v4f A, v4f B, v4f C)
{
	A = Mat4MVP * A;
	A = (1.0f / A.w) * A;
	A = Mat4ScreenSpace * A;

	B = Mat4MVP * B;
	B = (1.0f / B.w) * B;
	B = Mat4ScreenSpace * B;

	C = Mat4MVP * C;
	C = (1.0f / C.w) * C;
	C = Mat4ScreenSpace * C;

	v2f Tmp = {};
	v2f V0 = A.xy;
	v2f V1 = B.xy;
	v2f V2 = C.xy;

	v2f X = V1 - V0;
	v2f Y = V2 - V0;
	f32 SignedDoubleArea = Det(X, Y);
	b32 Swapped = false;
	if(SignedDoubleArea < 0)
	{
		Tmp = V1;
		V1 = V2;
		V2 = Tmp;
		Swapped = true;
	}

	s32 XMin = F32RoundToS32(Min3(V0.x, V1.x, V2.x));
	s32 YMin = F32RoundToS32(Min3(V0.y, V1.y, V2.y));
	s32 XMax = F32RoundToS32(Max3(V0.x, V1.x, V2.x));
	s32 YMax = F32RoundToS32(Max3(V0.y, V1.y, V2.y));

	for(s32 Y = YMin; Y < YMax; ++Y)
	{
		for(s32 X = XMin; X < XMax; ++X)
		{
			v2f P = {(f32)X, (f32)Y};

			f32 E01 = Det((V1 - V0), (P - V0));
			f32 E12 = Det((V2 - V1), (P - V1));
			f32 E20 = Det((V0 - V2), (P - V2));

			if((E01 > 0) && (E12 > 0) && (E20 > 0))
			{
				v3f Barycentric = {};
				if(Swapped)
				{
					Barycentric = BarycentricV2(V0, V2, V1, P);
				}
				else
				{
					Barycentric = BarycentricV2(V0, V1, V2, P);
				}

				PixelSet(AppBackBuffer, P, Barycentric);
			}
		}
	}
}

internal void
TriangleDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, triangle *Triangle, v4f Color)
{
	triangle Fragment = {};
	for(u32 Index = 0; Index < 3; Index++)
	{
		Fragment.Vertices[Index] = Mat4MVP * Triangle->Vertices[Index];
		Fragment.Vertices[Index] = (1.0f / Fragment.Vertices[Index].w) * Fragment.Vertices[Index];
		Fragment.Vertices[Index] = Mat4ScreenSpace * Fragment.Vertices[Index];

	}

	v2f Tmp = {};
	v2f V0 = Fragment.Vertices[0].xy;
	v2f V1 = Fragment.Vertices[1].xy;
	v2f V2 = Fragment.Vertices[2].xy;

	v2f X = V1 - V0;
	v2f Y = V2 - V0;
	f32 SignedDoubleArea = Det(X, Y);
	b32 Swapped = false;
	if(SignedDoubleArea < 0)
	{
		Tmp = V1;
		V1 = V2;
		V2 = Tmp;
		Swapped = true;
	}

	s32 XMin = F32RoundToS32(Min3(V0.x, V1.x, V2.x));
	s32 YMin = F32RoundToS32(Min3(V0.y, V1.y, V2.y));
	s32 XMax = F32RoundToS32(Max3(V0.x, V1.x, V2.x));
	s32 YMax = F32RoundToS32(Max3(V0.y, V1.y, V2.y));

	for(s32 Y = YMin; Y < YMax; ++Y)
	{
		for(s32 X = XMin; X < XMax; ++X)
		{
			v2f P = {(f32)X, (f32)Y};

			f32 E01 = Det((V1 - V0), (P - V0));
			f32 E12 = Det((V2 - V1), (P - V1));
			f32 E20 = Det((V0 - V2), (P - V2));

			if((E01 > 0) && (E12 > 0) && (E20 > 0))
			{
				v3f Barycentric = {};
				if(Swapped)
				{
					Barycentric = BarycentricV2(V0, V2, V1, P);
				}
				else
				{
					Barycentric = BarycentricV2(V0, V1, V2, P);
				}

				PixelSet(AppBackBuffer, P, Barycentric);
			}
		}
	}
}



internal void
GridDraw(app_back_buffer *BackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, v4f P)
{

	f32 Offset = 10.0f;

	v4f dZ = V4F(0.0f, 0.0f, -Offset, 0.0f);
	v4f PZ = P + dZ;

	for(s32 X = -5; X < 5; ++X)
	{
		P.x += (f32)X * 0.2f;
		PZ.x += (f32)X * 0.2f;

		
		v4f ScreenP = Mat4MVP * P;
		v4f ScreenPZ = Mat4MVP * PZ;

		ScreenP = (1.0f / ScreenP.w) * ScreenP;
		ScreenPZ = (1.0f / ScreenPZ.w) * ScreenPZ;

		ScreenP = Mat4ScreenSpace * ScreenP;
		ScreenPZ = Mat4ScreenSpace * ScreenPZ;

		v3f Color = {0.0f, 0.0f, 1.0f};
		LineDDADraw(BackBuffer, ScreenP.xy, ScreenPZ.xy, Color);
	}
}

inline f32
StringToFloat(char *String)
{
	f32 Result = (f32)atof(String);
	return(Result);
}

inline u32
CharTOU32(char C)
{
	u32 Result = 0;
	Result = Result * 10 + C - '0';
	return(Result);
}

inline b32
CharIsNum(char C)
{
	b32 Result = false;

	if((C >= '0') && (C <= '9'))
	{
		Result = true;
	}

	return(Result);
}

internal void
ParseV3VertexAttribute(u8 *Data, v3f *AttributeArray, u32 Count, u32 FloatCount)
{
	for(u32 Index = 0; Index < Count; ++Index)
	{
		f32 Floats[3] = {};
		for(u32 FloatIndex = 0; FloatIndex < FloatCount; ++FloatIndex)
		{
			f32 Value = StringToFloat((char *)Data);
			Floats[FloatIndex] = Value;
			while(*Data != ' ')
			{
				Data++;
			}
			Data++;
		}

		v3f Attribute = {Floats[0], Floats[1], Floats[2]};
		AttributeArray[Index] = Attribute;
	}
}

// TODO(Jusitn): Do the parsing in one routine
internal void
ParseV2VertexAttribute(u8 *Data, v2f *AttributeArray, u32 Count, u32 FloatCount)
{
	for(u32 Index = 0; Index < Count; ++Index)
	{
		f32 Floats[2] = {};
		for(u32 FloatIndex = 0; FloatIndex < FloatCount; ++FloatIndex)
		{
			f32 Value = StringToFloat((char *)Data);
			Floats[FloatIndex] = Value;
			while(*Data != ' ')
			{
				Data++;
			}
			Data++;
		}

		v2f Attribute = {Floats[0], Floats[1]};

		AttributeArray[Index] = Attribute;

	}
}

internal loaded_obj
DEBUGObjReadEntireFile(thread_context *Thread, char *FileName, memory_arena *Arena,
		debug_platform_file_read_entire_func *DEBUGPlatformReadEntireFile)
{
	loaded_obj Result = {};

	debug_file_read_result ObjFile = DEBUGPlatformReadEntireFile(Thread, FileName);
	if(ObjFile.Size != 0)
	{
		Result.Memory = (u8 *)ObjFile.Memory;
		Result.Size = ObjFile.Size;

		u32 VertexCount = 0;
		u32 TextureCoordCount = 0;
		u32 NormalCount = 0;
		u32 FaceCount = 0;

		u32 IndexCount = 0;

		char v = 'v';
		char Space = ' ';
		char t = 't';
		char n = 'n';
		char f = 'f';

		u32 FirstVertexOffset = 0;
		u32 FirstTextureOffset = 0;
		u32 FirstNormalOffset = 0;
		u32 FirstFaceOffset = 0;

		u32 FaceRows = 0;
		u32 FaceCols = 0;

		u32 FilePosition = 0;

		u8 *Content = Result.Memory;
		mesh *Mesh = &Result.Mesh;
		while(*Content++)
		{
			char Current = *Content;
			char Next = *(Content + 1);
			if(Next == '\0')
			{
				ASSERT((Current > 0) && (Current < 127) && (Current == '\n'));
				break;
			}

			if((Current == v) && (Next == Space))
			{
				if(FirstVertexOffset == 0)
				{
					FirstVertexOffset = FilePosition + 3;
				}
				Mesh->VertexCount++;
			}

			if((Current == t) && (Next == Space))
			{
				if(FirstTextureOffset == 0)
				{
					FirstTextureOffset = FilePosition + 4;
				}
				Mesh->TexCoordCount++;
			}

			if((Current == n) && (Next == Space))
			{
				if(FirstNormalOffset == 0)
				{
					FirstNormalOffset = FilePosition + 4;
				}
				Mesh->NormalCount++;
			}

			if((Current == f) && (Next == Space))
			{
				if(FirstFaceOffset == 0)
				{
					FirstFaceOffset = FilePosition + 3;
				}

				if(FaceRows == 0)
				{
					// NOTE(Justin): To find the number of FaceCols count the number of
					// spaces in the first face row.
					for(char *C = (char *)Content; *C != '\n'; C++)
					{
						if(*C == Space)
						{
							FaceCols++; 
						}
					}

				}

				FaceRows++;
			}

			FilePosition++;
		}

		FaceCount = 3 * FaceRows * FaceCols;
		Mesh->FaceCount = FaceCount;

		Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, v3f);
		Mesh->TexCoords = PushArray(Arena, Mesh->TexCoordCount, v2f);
		Mesh->Normals = PushArray(Arena, Mesh->NormalCount, v3f);
		Mesh->Faces = PushArray(Arena, Mesh->FaceCount, u32);

		Content = Result.Memory;
		Content += FirstVertexOffset;

		u8 *VertexData =  Content;
		ParseV3VertexAttribute(VertexData, Mesh->Vertices, Mesh->VertexCount, 3);

		Content = Result.Memory;
		Content += FirstTextureOffset;

		u8 *TextureData = Content;
		ParseV2VertexAttribute(TextureData, Mesh->TexCoords, Mesh->TexCoordCount, 2);

		Content = Result.Memory;
		Content += FirstNormalOffset;

		u8 *NormalData = Content;
		ParseV3VertexAttribute(NormalData, Mesh->Normals, Mesh->NormalCount, 3);

		Content = Result.Memory;
		Content += FirstFaceOffset;

		u8 *FaceData = Content;
		u32 FaceIndex = 0;
		char *C = (char *)FaceData;
		for(u32 Iteration = 0; Iteration < FaceCount; ++Iteration)
		{
			b32 Updated = false;
			u32 Num = 0;
			while(CharIsNum(*C))
			{
				Num = Num * 10 + (*C) - '0';
				C++;
				Updated = true;
			}

			if(Updated)
			{
				Mesh->Faces[FaceIndex++] = Num;
			}

			while(*C && !CharIsNum(*C))
			{
				C++;
			}
		}

		Mesh->IndicesCount = Mesh->FaceCount / 3;
		Mesh->Indices = PushArray(Arena, Mesh->IndicesCount, u32);
		for(u32 Index = 0; Index < Mesh->IndicesCount; ++Index)
		{
			u32 VertexIndex = Mesh->Faces[3 * Index] - 1;
			ASSERT(VertexIndex < UINT_MAX);
			Mesh->Indices[Index] = VertexIndex;
		}
	}


	return(Result);
}

// NOTE(Justin): BMP file format specification: https://www.fileformat.info/format/bmp/egff.htm

#pragma pack(push, 1)
struct bitmap_header
{
	u16 FileType;     /* File type, always 4D42h ("BM") */
	u32 FileSize;     /* Size of the file in bytes */
	u16 Reserved1;    /* Always 0 */
	u16 Reserved2;    /* Always 0 */
	u32 BitmapOffset; /* Starting position of image data in bytes */

	u32 Size;            /* Size of this header in bytes */
	s32  Width;           /* Image width in pixels */
	s32  Height;          /* Image height in pixels */
	u16  Planes;          /* Number of color planes */
	u16  BitsPerPixel;    /* Number of bits per pixel */
	/* Fields added for Windows 3.x follow this line */
	u32 Compression;     /* Compression methods used */
	u32 SizeOfBitmap;    /* Size of bitmap in bytes */
	s32  HorzResolution;  /* Horizontal resolution in pixels per meter */
	s32  VertResolution;  /* Vertical resolution in pixels per meter */
	u32 ColorsUsed;      /* Number of colors in the image */
	u32 ColorsImportant; /* Minimum number of important colors */
};
#pragma pack(pop)


#define BITMAP_BYTES_PER_PIXEL 4
internal loaded_bitmap
DEBUGBitmapReadEntireFile(thread_context *Thread, char *FileName, debug_platform_file_read_entire_func *DEBUGPlatformReadEntireFile)
{
	loaded_bitmap Result = {};
	debug_file_read_result File = DEBUGPlatformReadEntireFile(Thread, FileName);
	if(File.Size != 0)
	{
		bitmap_header *Header = (bitmap_header *)File.Memory;

		u8 *PixelData = ((u8 *)File.Memory + Header->BitmapOffset);

		Result.Width = Header->Width;
		Result.Height = Header->Height;
		Result.Stride = BITMAP_BYTES_PER_PIXEL * Header->Width;
		Result.Memory = PixelData;
	}

	return(Result);
}

internal void
BitmapDraw(app_back_buffer *AppBackBuffer, loaded_bitmap *Bitmap, v2f P)
{
	s32 XMin = F32RoundToS32(P.x);
	s32 YMin = F32RoundToS32(P.y);
	s32 XMax = F32RoundToS32(P.x + (f32)Bitmap->Width);
	s32 YMax = F32RoundToS32(P.y + (f32)Bitmap->Height);

	if(XMin < 0)
	{
		XMin = 0;
	}
	if(XMax > AppBackBuffer->Width)
	{
		XMax = AppBackBuffer->Width;
	}
	if(YMin < 0)
	{
		YMin = 0;
	}
	if(YMax > AppBackBuffer->Height)
	{
		YMax = AppBackBuffer->Height;
	}

	u8 *DestRow = (u8 *)AppBackBuffer->Memory + YMin * AppBackBuffer->Stride + XMin * BITMAP_BYTES_PER_PIXEL;
	u8 *SrcRow = (u8 *)Bitmap->Memory;

	for(s32 Y = YMin; Y < YMax; ++Y)
	{
		u32 *Dest = (u32 *)DestRow;
		u32 *Src = (u32 *)SrcRow;
		for(s32 X = XMin; X < XMax; ++X)
		{
			*Dest++ = *Src++;
		}

		DestRow += AppBackBuffer->Stride;
		SrcRow += Bitmap->Stride;
	}
}

internal void
CameraUpdate(app_state *AppState, app_back_buffer *BackBuffer, camera *Camera, f32 dMouseX, f32 dMouseY, f32 dt)
{

	f32 Sensitivity = 0.5f;

	f32 dX = dMouseX * dt * Sensitivity;
	f32 dY = dMouseY * dt * Sensitivity;

	f32 NewYaw = Camera->Yaw + dX;
	f32 NewPitch = Camera->Pitch + dY;

	Clamp(-90.0f, NewPitch, 90.0f);

	Camera->Yaw = NewYaw;
	Camera->Pitch = NewPitch;

	Camera->Direction.x = Cos(DEGREE_TO_RAD(Camera->Yaw)) * Cos(DEGREE_TO_RAD(Camera->Pitch));
	Camera->Direction.y = Sin(DEGREE_TO_RAD(Camera->Pitch));
	Camera->Direction.z = Sin(DEGREE_TO_RAD(Camera->Yaw)) * Cos(DEGREE_TO_RAD(Camera->Pitch));

	Camera->Direction = Normalize(Camera->Direction);
}

internal void
CameraInit(camera *Camera)
{
	Camera->Pos = {0.0f, 0.0f, 3.0f};
	Camera->Yaw = -90.0f;
	Camera->Pitch = 0.0f;

	Camera->Direction.x = Cos(DEGREE_TO_RAD(Camera->Yaw)) * Cos(DEGREE_TO_RAD(Camera->Pitch));
	Camera->Direction.y = Sin(DEGREE_TO_RAD(Camera->Pitch));
	Camera->Direction.z = Sin(DEGREE_TO_RAD(Camera->Yaw)) * Cos(DEGREE_TO_RAD(Camera->Pitch));
}

internal void
MeshVertexPositionsDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, mesh *Mesh)
{
	v2f Dim = V2F(1.0f);
	for(u32 Index = 0; Index < Mesh->VertexCount; ++Index)
	{
		v4f Vertex = V4FCreateFromV3F(Mesh->Vertices[Index], 1.0f);

		Vertex = Mat4MVP * Vertex;
		Vertex = (1.0f / Vertex.w) * Vertex;
		Vertex = Mat4ScreenSpace * Vertex;

		RectangleDraw(AppBackBuffer, Vertex.xy - Dim, Vertex.xy + Dim, V3F(1.0f));
	}
}

internal void
MeshWireFrameDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, mesh *Mesh)
{
	v3f Color = V3F(1.0f);
	for(u32 Index = 0; Index < Mesh->IndicesCount; Index += 4)
	{
		v4f V0 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index]], 1.0f);
		v4f V1 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index + 1]], 1.0f);
		v4f V2 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index + 2]], 1.0f);
		v4f V3 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index + 3]], 1.0f);

		V0 = Mat4MVP * V0;
		V1 = Mat4MVP * V1;
		V2 = Mat4MVP * V2;
		V3 = Mat4MVP * V3;

		V0 = (1.0f / V0.w) * V0;
		V1 = (1.0f / V1.w) * V1;
		V2 = (1.0f / V2.w) * V2;
		V3 = (1.0f / V3.w) * V3;

		V0 = Mat4ScreenSpace * V0;
		V1 = Mat4ScreenSpace * V1;
		V2 = Mat4ScreenSpace * V2;
		V3 = Mat4ScreenSpace * V3;

		LineDDADraw(AppBackBuffer, V0.xy, V1.xy, Color);
		LineDDADraw(AppBackBuffer, V1.xy, V2.xy, Color);
		LineDDADraw(AppBackBuffer, V2.xy, V0.xy, Color);
		LineDDADraw(AppBackBuffer, V2.xy, V3.xy, Color);
		LineDDADraw(AppBackBuffer, V3.xy, V0.xy, Color);
	}
}

internal void
MeshDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, mesh *Mesh)
{
	for(u32 Index = 0; Index < Mesh->IndicesCount; Index += 4)
	{
		v4f V0 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index]], 1.0f);
		v4f V1 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index + 1]], 1.0f);
		v4f V2 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index + 2]], 1.0f);
		v4f V3 = V4FCreateFromV3F(Mesh->Vertices[Mesh->Indices[Index + 3]], 1.0f);

		TriangleDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, V0, V1, V2);
		TriangleDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, V0, V3, V2);
		//TriangleDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, V0, V2, V3);
	}
}

extern "C" APP_UPDATE_AND_RENDER(app_update_and_render)
{
	app_state *AppState = (app_state *)AppMemory->PermanentStorage;
	
	ASSERT(sizeof(AppState) <= AppMemory->PermanentStorageSize);
	if(!AppMemory->IsInitialized)
	{
		// NOTE(Justin): The starting direction paramters product a direction of
		// (0, 0, -1) which is correct

		ArenaInitialize(&AppState->WorldArena, AppMemory->PermanentStorageSize - sizeof(app_state),
										 (u8 *)AppMemory->PermanentStorage + sizeof(app_state));

		AppState->Cube = DEBUGObjReadEntireFile(Thread,  "models/cube.obj", &AppState->WorldArena, AppMemory->debug_platform_file_read_entire);
		//AppState->Cube = DEBUGObjReadEntireFile(Thread,  "models/untitled.obj", &AppState->WorldArena, AppMemory->debug_platform_file_read_entire);
		AppState->Suzanne = DEBUGObjReadEntireFile(Thread,  "models/suzanne.obj", &AppState->WorldArena, AppMemory->debug_platform_file_read_entire);
		AppState->Test = DEBUGBitmapReadEntireFile(Thread, "structured_art.bmp", AppMemory->debug_platform_file_read_entire);



		camera *Camera = &AppState->Camera;
		CameraInit(Camera);

		AppState->MapToCamera = Mat4CameraMap(Camera->Pos, Camera->Pos + Camera->Direction);
		//AppState->MapToCamera = Mat4CameraMapV2(Camera->Pos, Camera->Right, Camera->Up, Camera->Direction);
		AppState->CameraIndex = 0;

		f32 FOV = DEGREE_TO_RAD(45.0f);
		f32 AspectRatio = (f32)AppBackBuffer->Width / (f32)AppBackBuffer->Height;
		f32 ZNear = 0.1f;
		f32 ZFar = 100.0f;

		AppState->MapToPersp = Mat4PerspectiveGL(FOV, AspectRatio, ZNear, ZFar);

		AppState->MapToScreenSpace = 
			Mat4ScreenSpaceMap(AppBackBuffer->Width, AppBackBuffer->Height);

		AppState->Triangle.Vertices[0] = V4F(-0.5f, -0.5f, 0.0f, 1.0f);
		AppState->Triangle.Vertices[1] = V4F(0.0f, 0.0f, 0.0f, 1.0f);
		AppState->Triangle.Vertices[2] = V4F(0.5f, -0.5f, 0.0f, 1.0f);

		AppState->Quad.Pos = V4F(10.0f, 0.0f, -2.0f, 1.0f);
		AppState->Quad.Vertices[0] = V4F(1.0f, 0.0f, 0.0f, 1.0f);
		AppState->Quad.Vertices[1] = V4F(1.0f, 1.0f, 0.0f, 1.0f);
		AppState->Quad.Vertices[2] = V4F(-1.0f, 1.0f, 0.0f, 1.0f);
		AppState->Quad.Vertices[3] = V4F(-1.0f, 0.0f, 0.0f, 1.0f);

		AppMemory->IsInitialized = true;
	}

	f32 dt = AppInput->dtForFrame;

	camera *Camera = &AppState->Camera;

	app_keyboard_controller *KeyBoardController = &AppInput->KeyboardController;


	// TODO(Justin): What do we update first? The camera's position or
	// orientation. How does updating the orientation/position affect the other?
	v3f dP = {};
	v3f Target = Camera->Pos + Camera->Direction;
	f32 CameraSpeed = 5.0f;
	if(KeyBoardController->W.EndedDown)
	{
		dP += dt * Camera->Direction;
	}

	if(KeyBoardController->S.EndedDown)
	{
		dP -= dt * Camera->Direction;
	}
	if(KeyBoardController->A.EndedDown)
	{
		dP += dt * Normalize(Cross(YAxis(), Camera->Direction));
	}
	if(KeyBoardController->D.EndedDown)
	{
		dP -= dt * Normalize(Cross(YAxis(), Camera->Direction));
	}


	dP *= CameraSpeed;
	Camera->Pos += dP;

	CameraUpdate(AppState, AppBackBuffer, Camera, AppInput->dMouseX, AppInput->dMouseY, dt);

	AppState->MapToCamera = Mat4CameraMap(Camera->Pos, Camera->Pos + Camera->Direction);
	//AppState->MapToCamera = Mat4CameraMapV2(Camera->Pos, Camera->Right, Camera->Up, Camera->Direction);

	mat4 MapToCamera = AppState->MapToCamera;
	mat4 MapToWorld = Mat4WorldSpaceMap(V3F(0.0f, 0.0f, -30.0f));
	mat4 MapToPersp = AppState->MapToPersp;
	mat4 Mat4MVP = MapToPersp * MapToCamera * MapToWorld;
	mat4 Mat4ScreenSpace = Mat4ScreenSpaceMap(AppBackBuffer->Width, AppBackBuffer->Height);

	//
	// NOTE(Justin): Render
	//

	// Background
	u32 *Pixel = (u32 *)AppBackBuffer->Memory;
	for(s32 Y = 0; Y < AppBackBuffer->Height; Y++)
	{
		for(s32 X = 0; X < AppBackBuffer->Width; X++)
		{
			*Pixel++ = 0;
		}
	}


	mat4 XRotation = Mat4Identity();
	mat4 YRotation = Mat4YRotation(dt);
	mat4 ZRotation = Mat4Identity();
	mat4 R = ZRotation * YRotation * XRotation;

	loaded_obj *Model = &AppState->Cube;
	mesh *Mesh = &Model->Mesh;
	for(u32 Index = 0; Index < Mesh->VertexCount; ++Index)
	{
		Mesh->Vertices[Index] = R * Mesh->Vertices[Index];
	}
	MeshDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, Mesh);
	//MeshWireFrameDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, Mesh);
}
