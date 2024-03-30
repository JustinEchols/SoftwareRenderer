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
	// TODO(Justin): Bounds checking
	s32 x_min = F32RoundToU32(Min.x);
	s32 x_max = F32RoundToU32(Max.x);

	s32 y_min = F32RoundToU32(Min.y);
	s32 y_max = F32RoundToU32(Max.y);

	if(x_min < 0)
	{
		x_min = 0;
	}
	if(x_max > AppBackBuffer->Width)
	{
		x_max = AppBackBuffer->Width;
	}
	if(y_min < 0)
	{
		y_min = 0;
	}
	if(y_max > AppBackBuffer->Height)
	{
		y_max = AppBackBuffer->Height;
	}

	u32 color = ColorConvertV3fToU32(Color);

	u8 * pixel_row = (u8 *)AppBackBuffer->Memory + AppBackBuffer->Stride * y_min + AppBackBuffer->BytesPerPixel * x_min;
	for(s32 y = y_min; y < y_max; y++)
	{
		u32 *pixel = (u32 *)pixel_row;
		for(s32 x = x_min; x < x_max; x++)
		{
			*pixel++ = color;
		}
		pixel_row += AppBackBuffer->Stride;
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
TriangleDraw(app_back_buffer *AppBackBuffer, triangle *Triangle)
{
	v3f Tmp = {};
	v3f Min = Triangle->Vertices[0].xyz;
	v3f Mid = Triangle->Vertices[1].xyz;
	v3f Max = Triangle->Vertices[2].xyz;

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


struct loaded_obj
{
	char *file_name;
	u8 *Memory;
	u32 Size;
};

struct mesh_attributes
{
	loaded_obj *Asset;

	u32 *indices;
	v3f *Vertices;
	v2f *Tex_coords;
	v3f *Normals;
	u32 *faces;

	u32 vertex_count;
	u32 tex_coord_count;
	u32 normal_count;
	u32 face_count;

	u32 face_Stride;
	u32 face_index_count;
};


#if 0
internal mesh_attributes *
model_process_file_contents(Memory_arena *Arena, loaded_obj ObjFile)
{
	mesh_attributes *Result = push_struct(Arena, mesh_attributes);
	char *contents = (char *)ObjFile.Memory;
	ASSERT(contents > 0);
	if(contents) {
		// First count each of the attributes st can push mesh attributes
		char v = 'v';
		char space = ' ';
		char t = 't';
		char n = 'n';
		char f = 'f';

		// Offsets from the start of the file to the start of an atrribute
		u32 first_vert_index = 0;
		u32 first_tex_index = 0;
		u32 first_normal_index = 0;
		u32 first_face_index = 0;

		u32 face_rows = 0;
		u32 attribute_location = 0;

		while (*contents++) {
			char char_at = *contents;
			char char_next = *(contents + 1);

			if(char_next == '\0') {
				ASSERT((char_at > 0) && (char_at < 127) && (char_at == '\n'));
				break;
			}

			if((char_at == v) && (char_next == space)) {
				if(first_vert_index == 0) {
					first_vert_index = attribute_location + 3;
				}
				Result->vertex_count++;
			}
			if((char_at == v) && (char_next == n)) {
				if(first_normal_index == 0) {
					first_normal_index = attribute_location + 4;
				}
				Result->normal_count++;
			}
			if((char_at == v) && (char_next == t)) {
				if(first_tex_index == 0) {
					first_tex_index = attribute_location + 4;
				}
				Result->tex_coord_count++;
			}
			if((char_at == f) && (char_next == space)) {
				if(first_face_index == 0) {
					first_face_index = attribute_location + 3;
				}
				face_rows++;
			}
			attribute_location++;
		}
	}
	Result->Vertices = push_array(Arena, Result->vertex_count, v3f);
	ontents = ObjFile.Memory;
	contents += first_vert_index;
	char *vert = contents;

	return(Result);
}
#endif
internal loaded_obj
DEBUGModelLoad(thread_context *Thread, debug_platform_file_read_entire_func *FileReadEntire, char *file_name)
{
	loaded_obj Result = {};
	debug_file_read_result FileReadResult = FileReadEntire(Thread, file_name);

	ASSERT(FileReadResult.Size > 0);

	if(FileReadResult.Size > 0)
	{
		Result.Memory = (u8 *)FileReadResult.Memory;
		Result.Size = FileReadResult.Size;
	}

	return(Result);
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

	Camera->Direction.x = cosf(DegreeToRad(Camera->Yaw)) * cosf(DegreeToRad(Camera->Pitch));
	Camera->Direction.y = sinf(DegreeToRad(Camera->Pitch));
	Camera->Direction.z = sinf(DegreeToRad(Camera->Yaw)) * cosf(DegreeToRad(Camera->Pitch));

	Camera->Direction = Normalize(Camera->Direction);
}

extern "C" APP_UPDATE_AND_RENDER(app_update_and_render)
{

	app_state *AppState = (app_state *)AppMemory->PermanentStorage;
	
	ASSERT(sizeof(AppState) <= AppMemory->PermanentStorageSize);
	if(!AppMemory->IsInitialized)
	{
		// NOTE(Justin): The starting direction paramters product a direction of
		// (0, 0, -1) which is correct

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



	v3f Shift = {};
	v3f Target = V3F(0.0f, 0.0f, -1.0f);
	if(KeyBoardController->W.EndedDown)
	{
		Shift = {0.0f, 0.0f, -1.0f * dt};
		Camera->Pos += Shift;
	}

	if(KeyBoardController->S.EndedDown)
	{
		Shift = {0.0f, 0.0f, 1.0f * dt};
		Camera->Pos += Shift;
	}
	if(KeyBoardController->A.EndedDown)
	{
		Shift = {-1.0f * dt, 0.0f, 0.0f};
		Camera->Pos += -dt * Normalize(Cross(Target, YAxis()));
	}
	if(KeyBoardController->D.EndedDown)
	{
		Camera->Pos += dt * Normalize(Cross(Target, YAxis()));
	}
	if(KeyBoardController->Up.EndedDown)
	{
		Shift = dt * Target;
		Camera->Pos += Shift;
	}
	if(KeyBoardController->Down.EndedDown)
	{
		Shift = -dt * Target;
		Camera->Pos += Shift;
	}

	CameraUpdate(AppState, AppBackBuffer, Camera, AppInput->dMouseX, AppInput->dMouseY, dt);



	AppState->MapToCamera = Mat4CameraMap(Camera->Pos, Camera->Pos + Camera->Direction);
	mat4 MapToCamera = AppState->MapToCamera;

	mat4 MapToWorld = Mat4WorldSpaceMap(AppState->Triangle.Pos.xyz);
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

	// Triangle
	mat4 RotateY = Mat4YRotation((dt * 2.0f * PI32 / 4.0f));
	triangle *Triangle = &AppState->Triangle;
	triangle Fragment;
	for(u32 i = 0; i < 3; i++)
	{
		Triangle->Vertices[i] = RotateY * Triangle->Vertices[i];

		Fragment.Vertices[i] = Mat4MVP * Triangle->Vertices[i];
		Fragment.Vertices[i] = (1.0f / Fragment.Vertices[i].w) * Fragment.Vertices[i];
		Fragment.Vertices[i] = Mat4ScreenSpace * Fragment.Vertices[i];

	}

	TriangleDraw(AppBackBuffer, &Fragment);

	AxisDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, V4F(0.0f, 0.0, 0.0, 1.0f));
	//GridDraw(AppBackBuffer, Mat4MVP, Mat4ScreenSpace, V4F(0.0f, 0.0f, 0.0f, 1.0f));
}
