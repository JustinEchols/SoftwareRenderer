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
	int x = (int)(PixelPos.x + 0.5f);
	int y = (int)(PixelPos.y + 0.5f);

	if((x < 0) || (x >= AppBackBuffer->Width))
	{
		return;
	}
	if((y < 0) || (y >= AppBackBuffer->Height))
	{
		return;
	}

	u32 color = ColorConvertV3fToU32(Color);

	u8 *pixel_row = (u8 *)AppBackBuffer->Memory + AppBackBuffer->Stride * y + AppBackBuffer->BytesPerPixel * x;
	u32 *pixel = (u32 *)pixel_row;
	*pixel = color;
}

internal void
LineDDADraw(app_back_buffer *AppBackBuffer, v2f P1, v2f P2, v3f Color)
{
	v2f Diff = P2 - P1;

	s32 dx = F32RoundToS32(Diff.x);
	s32 dy = F32RoundToS32(Diff.y);

	s32 pixel_count, pixel_index;

	if(ABS(dx) > ABS(dy))
	{
		pixel_count = ABS(dx);
	}
	else
	{
		pixel_count = ABS(dy);
	}

	v2f Increment = {(f32)dx / (f32)pixel_count, (f32)dy / (f32)pixel_count};
	v2f PixelPos = P1;
	PixelSet(AppBackBuffer, PixelPos, Color);
	for(pixel_index = 0; pixel_index < pixel_count; pixel_index++)
	{
		PixelPos += Increment;
		PixelSet(AppBackBuffer, PixelPos, Color);
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

internal v4f
LerpPoints(v4f P1, v4f P2, f32 t)
{
	v4f Result = (1.0f - t) * P1 + t * P2;
	return(Result);
}

internal v3f
LerpColor(v3f ColorA, v3f ColorB, f32 t)
{
	v3f Result = (1.0f - t) * ColorA + t * ColorB;
	return(Result);
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

internal edge
EdgeCreateFromV3f(v3f VertexMin, v3f VertexMax)
{
	edge Result;
	Result.YStart = F32RoundToS32(VertexMin.y);
	Result.YEnd = F32RoundToS32(VertexMax.y);

	f32 YDist = VertexMax.y - VertexMin.y;
	f32 XDist = VertexMax.x - VertexMin.x;

	Result.XStep = (f32)XDist / (f32)YDist;

	// Distance between real y and first scanline
	f32 YPrestep = Result.YStart - VertexMin.y;

	Result.X = VertexMin.x + YPrestep * Result.XStep;
	return(Result);
}

internal void
ScanlineDraw(app_back_buffer *AppBackBuffer, edge Left, edge Right, s32 ScanLine)
{
	s32 XMin = (s32)ceil(Left.X);
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
	f32 c_beta = 1.0f / (Dot(E1, E1) * Dot(E2, E2) - SQUARE(Dot(E1, E2)));
	Beta = c_beta * Beta; 

	Gamma = (Dot(E1, E1) * E2 - Dot(E1, E2) * E1); 
	f32 c_gamma = 1.0f / (Dot(E1, E1) * Dot(E2, E2) - SQUARE(Dot(E1, E2)));
	Gamma = c_gamma * Gamma;

	Result.x = Dot(Beta, F);
	Result.y = Dot(Gamma, F);
	Result.z = 1 - Result.x - Result.y;

	return(Result);
}

// TODO(Justin): Clean this function up.
internal void
TriangleDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, triangle *Triangle)
{
	triangle Fragment = {};
	for(u32 Index = 0; Index < 3; Index++)
	{
		Fragment.Vertices[Index] = Mat4MVP * Triangle->Vertices[Index];
		Fragment.Vertices[Index] = (1.0f / Fragment.Vertices[Index].w) * Fragment.Vertices[Index];
		Fragment.Vertices[Index] = Mat4ScreenSpace * Fragment.Vertices[Index];

	}

	v3f Tmp = {};
	v3f Min = Fragment.Vertices[0].xyz;
	v3f Mid = Fragment.Vertices[1].xyz;
	v3f Max = Fragment.Vertices[2].xyz;

	if(Min.y > Max.y)
	{
		Tmp = Min;
		Min = Max;
		Max = Tmp;
	}
	if(Mid.y >  Max.y)
	{
		Tmp = Mid;
		Mid = Max;
		Max = Tmp;
	}
	if(Min.y > Mid.y)
	{
		Tmp = Min;
		Min = Mid;
		Mid = Tmp;
	}

	edge BottomToTop = EdgeCreateFromV3f(Min, Max);
	edge MiddleToTop = EdgeCreateFromV3f(Mid, Max);
	edge BottomToMiddle = EdgeCreateFromV3f(Min, Mid);

	v2f V0 = Min.xy - Max.xy;
	v2f V1 = Mid.xy - Max.xy;

	f32 AreaDoubleSigned = Det(V0, V1);

	b32 OrientedRight;
	if(AreaDoubleSigned > 0)
	{
		// BottomToTop is on the left
		// Two edges on the right
		OrientedRight = true;
	}
	else
	{
		// BottomToTop is on the right 
		// Two edges on the left 
		OrientedRight = false;
	}

	edge Left = BottomToTop;
	edge Right = BottomToMiddle;
	if(!OrientedRight)
	{
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	int YStart = BottomToMiddle.YStart;
	int YEnd = BottomToMiddle.YEnd;

	v3f P = Min;

	v3f Color = {};
	for(int j = YStart; j < YEnd; j++)
	{
		// For each scanline
		P.x = Left.X;
		int XStart = F32RoundToS32(Left.X);
		int XEnd = F32RoundToS32(Right.X);
		for(int i = XStart; i < XEnd; i++)
		{
			// Get the barycentric cood. for each P in the scanline 
			// and set the corresponding pixel. 
			
			Color = Barycentric(Min, Max, Mid, P);
			PixelSet(AppBackBuffer, P.xy, Color);
			P.x++;
		}
		// Increment Left and Right to proper x -values of next scanline.
		// Increment point P y - value to next scanline.
		Left.X += Left.XStep;
		Right.X += Right.XStep;
		P.y++;
	}

	Left = BottomToTop;
	Right = MiddleToTop;

	// Offset the starting x value so that it is at the
	// correct x value to render the top half of the triangle
	Left.X += (MiddleToTop.YStart - BottomToTop.YStart) * Left.XStep;

	if(!OrientedRight)
	{
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	YStart = MiddleToTop.YStart;
	YEnd = MiddleToTop.YEnd;

	//P = Min;
	P.y = (f32)YStart;
	for(int j = YStart; j < YEnd; j++)
	{
		P.x = Left.X;
		int XStart = F32RoundToS32(Left.X);
		int XEnd = F32RoundToS32(Right.X);
		for(int i = XStart; i < XEnd; i++)
		{
			Color = Barycentric(Min, Max, Mid, P);
			PixelSet(AppBackBuffer, P.xy, Color);
			P.x++;
		}
		Left.X += Left.XStep;
		Right.X += Right.XStep;
		P.y++;
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

internal mesh_attributes
DEBUGObjReadEntireFile(thread_context *Thread, char *FileName, memory_arena *Arena,
		debug_platform_file_read_entire_func *DEBUGPlatformReadEntireFile)
{
	mesh_attributes Result = {};
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

		u32 FilePosition = 0;

		u8 *Content = Result.Memory;
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
				Result.VertexCount++;
			}

			if((Current == t) && (Next == Space))
			{
				if(FirstTextureOffset == 0)
				{
					FirstTextureOffset = FilePosition + 4;
				}
				Result.TexCoordCount++;
			}

			if((Current == n) && (Next == Space))
			{
				if(FirstNormalOffset == 0)
				{
					FirstNormalOffset = FilePosition + 4;
				}
				Result.NormalCount++;
			}

			if((Current == f) && (Next == Space))
			{
				if(FirstFaceOffset == 0)
				{
					FirstFaceOffset = FilePosition + 3;
				}
				FaceRows++;
			}
			FilePosition++;
		}

		FaceCount = FaceRows * 3 * 4;
		Result.FaceCount = FaceCount;

		Result.Vertices = PushArray(Arena, Result.VertexCount, v3f);
		Result.TexCoords = PushArray(Arena, Result.TexCoordCount, v2f);
		Result.Normals = PushArray(Arena, Result.NormalCount, v3f);
		Result.Faces = PushArray(Arena, Result.FaceCount, u32);

		Content = Result.Memory;
		Content += FirstVertexOffset;

		u8 *VertexData =  Content;
		ParseV3VertexAttribute(VertexData, Result.Vertices, Result.VertexCount, 3);

		Content = Result.Memory;
		Content += FirstTextureOffset;

		u8 *TextureData = Content;
		ParseV2VertexAttribute(TextureData, Result.TexCoords, Result.TexCoordCount, 2);

		Content = Result.Memory;
		Content += FirstNormalOffset;

		u8 *NormalData = Content;
		ParseV3VertexAttribute(NormalData, Result.Normals, Result.NormalCount, 3);

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
				Result.Faces[FaceIndex++] = Num;
			}

			while(*C && !CharIsNum(*C))
			{
				C++;
			}
		}
	}

	return(Result);
}

#if 0
internal loaded_obj
DEBUGModelLoad(thread_context *Thread, debug_platform_file_read_entire_func *FileReadEntire, char *FileName)
{
	loaded_obj Result = {};
	debug_file_read_result FileReadResult = FileReadEntire(Thread, FileName);

	if(FileReadResult.Size > 0)
	{
		Result.Memory = (u8 *)FileReadResult.Memory;
		Result.Size = FileReadResult.Size;
	}

	return(Result);
}
#endif

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

	Camera->Direction.x = Cos(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Camera->Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));

	Camera->Direction = Normalize(Camera->Direction);
}

internal void
MeshVertexPositionsDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, mesh_attributes *Mesh)
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
CubeWireFrameDraw(app_back_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, mesh_attributes *CubeMesh)
{
	v2f Dim = V2F(1.0f);
	for(u32 Index = 0; Index < CubeMesh->VertexCount; ++Index)
	{
		v4f Vertex0 = V4FCreateFromV3F(CubeMesh->Vertices[Index], 1.0f);
		v4f Vertex1 = V4FCreateFromV3F(CubeMesh->Vertices[(Index + 1) % CubeMesh->VertexCount], 1.0f);

		Vertex0 = Mat4MVP * Vertex0;
		Vertex1 = Mat4MVP * Vertex1;
		Vertex0 = (1.0f / Vertex0.w) * Vertex0;
		Vertex1 = (1.0f / Vertex1.w) * Vertex1;
		Vertex0 = Mat4ScreenSpace * Vertex0;
		Vertex1 = Mat4ScreenSpace * Vertex1;

		LineDDADraw(AppBackBuffer, Vertex0.xy, Vertex1.xy, V3F(1.0f));
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

		AppState->CubeMesh = DEBUGObjReadEntireFile(Thread,  "models/cube.obj", &AppState->WorldArena, AppMemory->debug_platform_file_read_entire);
		AppState->SuzanneMesh = DEBUGObjReadEntireFile(Thread,  "models/suzanne.obj", &AppState->WorldArena, AppMemory->debug_platform_file_read_entire);

		camera *Camera = &AppState->Camera;
		Camera->Pos = {0.0f, 0.0f, 3.0f};
		Camera->Yaw = -90.0f;
		Camera->Pitch = 0.0f;
		Camera->Direction.x = cosf(DegreeToRad(Camera->Yaw)) * cosf(DegreeToRad(Camera->Pitch));
		Camera->Direction.y = sinf(DegreeToRad(Camera->Pitch));
		Camera->Direction.z = sinf(DegreeToRad(Camera->Yaw)) * cosf(DegreeToRad(Camera->Pitch));

		AppState->MapToCamera = Mat4CameraMap(Camera->Pos, Camera->Pos + Camera->Direction);
		AppState->CameraIndex = 0;

		f32 FOV = DegreeToRad(45.0f);
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
	v3f ddP = {};
	v3f Target = Camera->Pos + Camera->Direction;
	if(KeyBoardController->W.EndedDown)
	{
		ddP += {0.0f, 0.0f, -1.0f * dt};
	}

	if(KeyBoardController->S.EndedDown)
	{
		ddP += {0.0f, 0.0f, 1.0f * dt};
	}
	if(KeyBoardController->A.EndedDown)
	{
		ddP += {-1.0f * dt, 0.0f, 0.0f};
	}
	if(KeyBoardController->D.EndedDown)
	{
		ddP += dt * Normalize(Cross(Target, YAxis()));
	}
	if(KeyBoardController->Up.EndedDown)
	{
		ddP += dt * Target;
	}
	if(KeyBoardController->Down.EndedDown)
	{
		ddP += -dt * Target;
	}

	if((ddP.x != 0.0f) && (ddP.y != 0.0f) && (ddP.z != 0.0f))
	{
		ddP = Normalize(ddP);
	}

	ddP *= 8.0f;
	Camera->Pos += ddP;

	CameraUpdate(AppState, AppBackBuffer, Camera, AppInput->dMouseX, AppInput->dMouseY, dt);

	AppState->MapToCamera = Mat4CameraMap(Camera->Pos, Camera->Pos + Camera->Direction);

	mat4 MapToCamera = AppState->MapToCamera;
	mat4 MapToWorld = Mat4WorldSpaceMap(V3F(0.0f, 0.0f, -30.0f));
	mat4 MapToPersp = AppState->MapToPersp;
	mat4 Mat4MVP = MapToPersp * MapToCamera * MapToWorld;
	mat4 Mat4ScreenSpace = Mat4ScreenSpaceMap(AppBackBuffer->Width, AppBackBuffer->Height);

	//
	// NOTE(Justin): Render
	//

	// Background
	u32 *pixel = (u32 *)AppBackBuffer->Memory;
	for(s32 y = 0; y < AppBackBuffer->Height; y++)
	{
		for(s32 x = 0; x < AppBackBuffer->Width; x++)
		{
			*pixel++ = 0;
		}
	}

	MeshVertexPositionsDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, &AppState->SuzanneMesh);
}
