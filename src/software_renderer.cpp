#include "software_renderer.h"

internal v3f
color_rand_init()
{
	v3f Result;
	Result.r = ((f32)rand() / (f32)RAND_MAX);
	Result.g = ((f32)rand() / (f32)RAND_MAX);
	Result.b = ((f32)rand() / (f32)RAND_MAX);

	return(Result);
}

internal v4f
v4f_rand_init()
{
	v4f Result = {};

	Result.x = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
	Result.y = ((((f32)rand() / (f32)RAND_MAX) * 2.0f) - 1.0f);
	Result.z = (((f32)rand() / (f32)RAND_MAX) - 2.0f);
	Result.w = 1;

	return(Result);
}

internal triangle
triangle_rand_init()
{
	triangle Result = {};
	for(u32 i = 0; i < 3; i++) {
		Result.Vertices[i] = v4f_rand_init();
	}
	Result.Color = color_rand_init();
	return(Result);
}

internal u32
color_convert_v3f_to_u32(v3f Color)
{
	u32 Result = 0;

	u32 red		= (u32)(255.0f * Color.r);
	u32 green	= (u32)(255.0f * Color.g);
	u32 blue	= (u32)(255.0f * Color.b);
	Result		= ((red << 16) | (green << 8) | (blue << 0));

	return(Result);
}

internal void
pixel_set(app_back_buffer *AppBackBuffer, v2f PixelPos, v3f Color)
{
	int x = (int)(PixelPos.x + 0.5f);
	int y = (int)(PixelPos.y + 0.5f);

	if ((x < 0) || (x >= AppBackBuffer->width)) {
		return;
	}
	if ((y < 0) || (y >= AppBackBuffer->height)) {
		return;
	}

	u32 color = color_convert_v3f_to_u32(Color);

	u8 *pixel_row = (u8 *)AppBackBuffer->memory + AppBackBuffer->stride * y + AppBackBuffer->bytes_per_pixel * x;
	u32 *pixel = (u32 *)pixel_row;
	*pixel = color;
}

internal void
line_draw_dda(app_back_buffer *AppBackBuffer, v2f P1, v2f P2, v3f Color)
{
	v2f Diff = P2 - P1;

	s32 dx = f32_round_to_s32(Diff.x);
	s32 dy = f32_round_to_s32(Diff.y);

	s32 pixel_count, pixel_index;

	if (ABS(dx) > ABS(dy)) {
		pixel_count = ABS(dx);
	} else {
		pixel_count = ABS(dy);
	}

	v2f Increment = {(f32)dx / (f32)pixel_count, (f32)dy / (f32)pixel_count};
	v2f PixelPos = P1;
	pixel_set(AppBackBuffer, PixelPos, Color);
	for (pixel_index = 0; pixel_index < pixel_count; pixel_index++) {
		PixelPos += Increment;
		pixel_set(AppBackBuffer, PixelPos, Color);
	}
}

internal void
circle_draw(app_back_buffer *AppBackBuffer, circle Circle, v3f Color)
{
#if 0
	v2f Radius = {Circle.radius, Circle.radius};
	v2f Min = Circle.Center.xy - Radius;
	v2f Max = Circle.Center.xy + Radius;
#endif

	u32 x_min = f32_round_to_u32(Circle.Center.x - Circle.radius);
	u32 x_max = f32_round_to_u32(Circle.Center.x + Circle.radius);

	u32 y_min = f32_round_to_u32(Circle.Center.y - Circle.radius);
	u32 y_max = f32_round_to_u32(Circle.Center.y + Circle.radius);

	f32 distance_squared = 0.0f;
	f32 radius_sqaured = SQUARE(Circle.radius);

	v2f Test;
	for (u32 y = y_min; y < y_max; y++)  {
		for (u32 x = x_min; x < x_max; x++)  {

			v2f PixelCurrent = {(f32)x, (f32)y};
			Test = Circle.Center.xy - PixelCurrent;

			distance_squared = v2f_innerf(Test, Test);
			if (distance_squared <= radius_sqaured) {
				pixel_set(AppBackBuffer, Test, Color);
			}
		}
	}
}

internal v4f
lerp_points(v4f P1, v4f P2, f32 t)
{
	v4f Result = (1.0f - t) * P1 + t * P2;
	return(Result);
}

internal v3f
lerp_color(v3f ColorA, v3f ColorB, f32 t)
{
	v3f Result = (1.0f - t) * ColorA + t * ColorB;
	return(Result);
}

internal void
rectangle_draw(app_back_buffer *AppBackBuffer, rectangle R, v3f Color)
{
	// TODO(Justin): Bounds checking
	s32 x_min = f32_round_to_u32(R.Min.x);
	s32 x_max = f32_round_to_u32(R.Max.x);

	s32 y_min = f32_round_to_u32(R.Min.y);
	s32 y_max = f32_round_to_u32(R.Max.y);

	if (x_min < 0) {
		x_min = 0;
	}
	if (x_max > AppBackBuffer->width) {
		x_max = AppBackBuffer->width;
	}
	if (y_min < 0) {
		y_min = 0;
	}
	if (y_max > AppBackBuffer->height) {
		y_max = AppBackBuffer->height;
	}

	u32 color = color_convert_v3f_to_u32(Color);

	u8 * pixel_row = (u8 *)AppBackBuffer->memory + AppBackBuffer->stride * y_min + AppBackBuffer->bytes_per_pixel * x_min;
	for (s32 y = y_min; y < y_max; y++) {
		u32 *pixel = (u32 *)pixel_row;
		for (s32 x = x_min; x < x_max; x++) {
			*pixel++ = color;
		}
		pixel_row += AppBackBuffer->stride;
	}
}

internal void
axis_draw(app_back_buffer *AppBackBuffer, m4x4 M, v4f Position)
{
	v4f Position2 = {0.0f, 0.0f, Position.z - 1.0f, 1};
	v4f Position3 = {Position.x + 1.0f, 0.0f, Position.z, 1};
	v4f Position4 = {0.0f, Position.y + 1.0f, Position.z, 1};

	Position = M * Position;
	Position = (1.0f / Position.w) * Position;


	Position2 = M * Position2;
	Position2 = (1.0f / Position2.w) * Position2;

	v3f Color = {0.0f, 0.0f, 1.0f};
	line_draw_dda(AppBackBuffer, Position.xy, Position2.xy, Color);


	Position3 = M * Position3;
	Position3 = (1.0f / Position3.w) * Position3;

	Color = {1.0f, 0.0f, 0.0f};
	line_draw_dda(AppBackBuffer, Position.xy, Position3.xy, Color);


	Position4 = M * Position4;
	Position4 = (1.0f / Position4.w) * Position4;

	Color = {0.0f, 1.0f, 0.0f};
	line_draw_dda(AppBackBuffer, Position.xy, Position4.xy, Color);

}

internal edge
edge_create_from_v3f(v3f VertexMin, v3f VertexMax)
{
	edge Result;
	Result.y_start = f32_round_to_s32(VertexMin.y);
	Result.y_end = f32_round_to_s32(VertexMax.y);

	f32 y_dist = VertexMax.y - VertexMin.y;
	f32 x_dist = VertexMax.x - VertexMin.x;

	Result.x_step = (f32)x_dist / (f32)y_dist;

	// Distance between real y and first scanline
	f32 y_prestep = Result.y_start - VertexMin.y;

	Result.x = VertexMin.x + y_prestep * Result.x_step;
	return(Result);
}

internal void
scanline_draw(app_back_buffer *AppBackBuffer, edge Left, edge Right, s32 scanline)
{
	s32 x_min = (s32)ceil(Left.x);
	s32 x_max = (s32)ceil(Right.x);

	u8 *pixel_row = (u8 *)AppBackBuffer->memory + AppBackBuffer->stride * scanline + AppBackBuffer->bytes_per_pixel * x_min;
	u32 *pixel = (u32 *)pixel_row;
	for (s32 i = x_min; i < x_max; i++) {
		*pixel++ = 0xFFFFFFFF;
	}
}

internal v3f
barycentric_cood(v3f X, v3f Y, v3f Z, v3f P)
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

	Beta = (v3f_innerf(E2, E2) * E1 - v3f_innerf(E1, E2) * E2); 
	f32 c_beta = 1.0f / (v3f_innerf(E1, E1) * v3f_innerf(E2, E2) - SQUARE(v3f_innerf(E1, E2)));
	Beta = c_beta * Beta; 

	Gamma = (v3f_innerf(E1, E1) * E2 - v3f_innerf(E1, E2) * E1); 
	f32 c_gamma = 1.0f / (v3f_innerf(E1, E1) * v3f_innerf(E2, E2) - SQUARE(v3f_innerf(E1, E2)));
	Gamma = c_gamma * Gamma;

	Result.x = v3f_innerf(Beta, F);
	Result.y = v3f_innerf(Gamma, F);
	Result.z = 1 - Result.x - Result.y;

	return(Result);
}

// TODO(Justin): Clean this function up.
internal void
triangle_scan_interpolation(app_back_buffer *AppBackBuffer, triangle *Triangle)
{
	v3f Tmp = {};
	v3f Min = Triangle->Vertices[0].xyz;
	v3f Mid = Triangle->Vertices[1].xyz;
	v3f Max = Triangle->Vertices[2].xyz;

	if (Min.y > Max.y) {
		Tmp = Min;
		Min = Max;
		Max = Tmp;
	}
	if (Mid.y >  Max.y) {
		Tmp = Mid;
		Mid = Max;
		Max = Tmp;
	}
	if (Min.y > Mid.y) {
		Tmp = Min;
		Min = Mid;
		Mid = Tmp;
	}

	edge BottomToTop = edge_create_from_v3f(Min, Max);
	edge MiddleToTop = edge_create_from_v3f(Mid, Max);
	edge BottomToMiddle = edge_create_from_v3f(Min, Mid);


	f32 v0_x = Min.x - Max.x;
	f32 v0_y = Min.y - Max.y;

	f32 v1_x = Mid.x - Max.x;
	f32 v1_y = Mid.y - Max.y;

	f32 area_double_signed = v0_x * v1_y - v1_x * v0_y;

	b32 oriented_right;
	if (area_double_signed > 0) {
		// BottomToTop is on the left
		// Two edges on the right
		oriented_right = true;
	} else {
		// BottomToTop is on the right 
		// Two edges on the left 
		oriented_right = false;
	}

	edge Left = BottomToTop;
	edge Right = BottomToMiddle;
	if (!oriented_right) {
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	int y_start = BottomToMiddle.y_start;
	int y_end = BottomToMiddle.y_end;

	v3f P = Min;
	v3f Color = {};
	for (int j = y_start; j < y_end; j++) {
		// For each scanline
		P.x = Left.x;
		int x_start = f32_round_to_s32(Left.x);
		int x_end = f32_round_to_s32(Right.x);
		for (int i = x_start; i < x_end; i++) {
			// Get the barycentric cood. for each P in the scanline 
			// and set the corresponding pixel. 
			Color = barycentric_cood(Min, Max, Mid, P);
			pixel_set(AppBackBuffer, P.xy, Color);
			P.x++;
		}
		// Increment Left and Right to proper x -values of next scanline.
		// Increment point P y - value to next scanline.
		Left.x += Left.x_step;
		Right.x += Right.x_step;
		P.y++;
	}

	Left = BottomToTop;
	Right = MiddleToTop;

	// Offset the starting x value so that it is at the
	// correct x value to render the top half of the triangle
	Left.x += (MiddleToTop.y_start - BottomToTop.y_start) * Left.x_step;

	if (!oriented_right) {
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	y_start = MiddleToTop.y_start;
	y_end = MiddleToTop.y_end;

	//P = Min;
	P.y = (f32)y_start;
	for (int j = y_start; j < y_end; j++) {
		P.x = Left.x;
		int x_start = f32_round_to_s32(Left.x);
		int x_end = f32_round_to_s32(Right.x);
		for (int i = x_start; i < x_end; i++) {
			Color = barycentric_cood(Min, Max, Mid, P);
			pixel_set(AppBackBuffer, P.xy, Color);
			P.x++;
		}
		Left.x += Left.x_step;
		Right.x += Right.x_step;
		P.y++;
	}
}

internal void
camera_update(camera *Camera, app_mouse_controller *MouseController)
{
}

extern "C" APP_UPDATE_AND_RENDER(app_update_and_render)
{
	app_state *AppState = (app_state *)AppMemory->permanent_storage;

	// TODO(Justin): Assert
	if (!AppMemory->is_initialized) {
		v3f CameraPos = {0.0f, 0.0f, 1.0f};
		v3f CameraDirection = {0.0f, 0.0f, 1.0f};
		v3f CameraUp = {0.0f, 1.0f, 0.0f};

		v3f Camera2Pos = {1.0f, 1.0f, 1.0f};
		v3f Camera2Direction = {0.0f, 0.0f, 1.0f};
		v3f Camera2Up = {0.0f, 1.0f, 0.0f};

		v3f CameraAbovePos = {0.0f, 5.0f, 1.0f};
		v3f CameraAboveDirection = {0.0f, -1.0f, 0.0f};
		v3f CameraAboveUp = {0.0f, 0.0f, 1.0f};

		AppState->Cameras[0] = {CameraPos, CameraDirection, CameraUp};
		AppState->Cameras[1] = {Camera2Pos, Camera2Direction, Camera2Up};
		AppState->Cameras[2] = {CameraAbovePos, CameraAboveDirection, CameraAboveUp};

		AppState->CameraIndex = 0;

		f32 l = -1.0f;
		f32 r = 1.0f;
		f32 b = -1.0f;
		f32 t = 1.0f;
		f32 n = 1.0f;
		f32 f = 2.0f;


		AppState->MapToCamera = m4x4_camera_map_create(CameraPos, CameraDirection, CameraUp);
		AppState->MapToPersp = m4x4_perspective_projection_create(l , r, b, t, n, f);
		AppState->MapToScreenSpace = 
			m4x4_screen_space_map_create(AppBackBuffer->width, AppBackBuffer->height);

		AppState->Triangle.Pos = {0.0f, 0.0f, -5.0f, 1.0f};
		AppState->Triangle.Vertices[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		AppState->Triangle.Vertices[1] = {0.5f, 0.0f, 0.0f, 1.0f};
		AppState->Triangle.Vertices[2] = {0.0f, 0.5f, 0.0f, 1.0f};

		AppMemory->is_initialized = true;
	}

	f32 time_delta = AppInput->time_delta;

	camera *Camera;
	if (AppInput->KeyboardController.One.ended_down && (AppState->CameraIndex != 0)) {
		Camera = &AppState->Cameras[0];
		AppState->CameraIndex = 0;
	} else if (AppInput->KeyboardController.Two.ended_down && (AppState->CameraIndex != 1)) {
		Camera = &AppState->Cameras[1];
		AppState->CameraIndex = 1;
	} else if (AppInput->KeyboardController.Three.ended_down) {
		Camera = &AppState->Cameras[2];
		AppState->CameraIndex = 2;
	} else {
		Camera = &AppState->Cameras[AppState->CameraIndex];
	}

	// TODO(Justin): Genreal way of moving cameras.
	v3f Shift = {};
	if (AppInput->KeyboardController.W.ended_down) {
		Shift = {0.0f, 1.0f * time_delta, 0.0f};
		Camera->Pos += Shift;
	}
	if (AppInput->KeyboardController.S.ended_down) {
		Shift = {0.0f, -1.0f * time_delta, 0.0f};
		Camera->Pos += Shift;
	}
	if (AppInput->KeyboardController.A.ended_down) {
		Shift = {-1.0f * time_delta, 0.0f, 0.0f};
		Camera->Pos += Shift;
	}
	if (AppInput->KeyboardController.D.ended_down) {
		Shift = {1.0f * time_delta, 0.0f, 0.0f};
		Camera->Pos += Shift;
	}
	if (AppInput->KeyboardController.Up.ended_down) {
		Shift = {0.0f, 0.0f, -1.0f * time_delta};
		Camera->Pos += Shift;
	}
	if (AppInput->KeyboardController.Down.ended_down) {
		Shift = {0.0f, 0.0f, 1.0f * time_delta};
		Camera->Pos += Shift;
	}

	m4x4 MapToWorld = m4x4_world_space_map_create(AppState->Triangle.Pos.xyz);

	AppState->MapToCamera = m4x4_camera_map_create(Camera->Pos, Camera->Direction, Camera->Up);
	m4x4 MapToCamera = AppState->MapToCamera;

	m4x4 MapToScreenSpace = m4x4_screen_space_map_create(AppBackBuffer->width, AppBackBuffer->height);
	m4x4 MapToPersp = AppState->MapToPersp;

	m4x4 M = MapToScreenSpace * MapToPersp * MapToCamera * MapToWorld;

	//
	// NOTE(Justin): Render
	//

	u32 *pixel = (u32 *)AppBackBuffer->memory;
	for (s32 y = 0; y < AppBackBuffer->height; y++) {
		for (s32 x = 0; x < AppBackBuffer->width; x++) {
			*pixel++ = 0;
		}
	}

	m4x4 RotateY = m4x4_rotation_y_create((time_delta * 2.0f * PI32 / 4.0f));
	triangle *Triangle = &AppState->Triangle;
	triangle Fragment;
	for (u32 i = 0; i < 3; i++) {
		Triangle->Vertices[i] = RotateY * Triangle->Vertices[i];

		Fragment.Vertices[i] = M * Triangle->Vertices[i];
		Fragment.Vertices[i] = (1.0f / Fragment.Vertices[i].w) * Fragment.Vertices[i];
	}
	triangle_scan_interpolation(AppBackBuffer, &Fragment);

}
