#include "software_renderer.h"
#include "software_renderer_math.h"

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

	int dx = round_f32_to_s32(Diff.x);
	int dy = round_f32_to_s32(Diff.y);

	int steps, k;

	if (ABS(dx) > ABS(dy)) {
		steps = ABS(dx);
	} else {
		steps = ABS(dy);
	}

	v2f Increment = {(f32)dx / (f32)steps, (f32)dy / (f32)steps};
	v2f PixelPos = P1;

	pixel_set(AppBackBuffer, PixelPos, Color);
	for (k = 0; k < steps; k++) {
		PixelPos += Increment;
		pixel_set(AppBackBuffer, PixelPos, Color);
	}
}

internal void
circle_draw(app_back_buffer *AppBackBuffer, circle Circle, v3f Color)
{
	u32 x_min = round_f32_to_u32(Circle.Center.x - Circle.radius);
	u32 x_max = round_f32_to_u32(Circle.Center.x + Circle.radius);

	u32 y_min = round_f32_to_u32(Circle.Center.y - Circle.radius);
	u32 y_max = round_f32_to_u32(Circle.Center.y + Circle.radius);

	f32 distance_squared = 0.0f;
	f32 radius_sqaured = SQUARE(Circle.radius);

	f32 test_x, test_y;
	v2f test = {0.0f, 0.0f};
	for (u32 y = y_min; y < y_max; y++)  {
		for (u32 x = x_min; x < x_max; x++)  {
			test_x = Circle.Center.x - x;
			test_y = Circle.Center.y - y;

			distance_squared = SQUARE(test_x) + SQUARE(test_y);
			if (distance_squared <= radius_sqaured) {
				test = {(f32)x, (f32)y};
				pixel_set(AppBackBuffer, test, Color);
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
	//TODO: ASSERT t in [0, 1]

	v3f Result = (1.0f - t) * ColorA + t * ColorB;
	return(Result);
}



internal void
rectangle_draw(app_back_buffer *AppBackBuffer, rectangle R, v3f Color)
{
	// TODO(Justin): Bounds checking
	u32 x_min = round_f32_to_u32(R.Min.x);
	u32 x_max = round_f32_to_u32(R.Max.x);

	u32 y_min = round_f32_to_u32(R.Min.y);
	u32 y_max = round_f32_to_u32(R.Max.y);


	u32 color = color_convert_v3f_to_u32(Color);

	u8 * pixel_row = (u8 *)AppBackBuffer->memory + AppBackBuffer->stride * y_min + AppBackBuffer->bytes_per_pixel * x_min;
	for (u32 y = y_min; y < y_max; y++) {
		u32 *pixel = (u32 *)pixel_row;
		for (u32 x = x_min; x < x_max; x++) {
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
	Result.y_start = round_f32_to_s32(VertexMin.y);
	Result.y_end = round_f32_to_s32(VertexMax.y);

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

internal void
triangle_scan_interpolation(app_back_buffer *AppBackBuffer, triangle *Triangle)
{
	v3f tmp = {};
	v3f min = Triangle->Vertices[0].xyz;
	v3f mid = Triangle->Vertices[1].xyz;
	v3f max = Triangle->Vertices[2].xyz;

	if (min.y > max.y) {
		tmp = min;
		min = max;
		max = tmp;
	}
	if (mid.y >  max.y) {
		tmp = mid;
		mid = max;
		max = tmp;
	}
	if (min.y > mid.y) {
		tmp = min;
		min = mid;
		mid = tmp;
	}

	edge BottomToTop = edge_create_from_v3f(min, max);
	edge MiddleToTop = edge_create_from_v3f(mid, max);
	edge BottomToMiddle = edge_create_from_v3f(min, mid);


	f32 v0_x = min.x - max.x;
	f32 v0_y = min.y - max.y;

	f32 v1_x = mid.x - max.x;
	f32 v1_y = mid.y - max.y;

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
	b32 which_side;
	if (oriented_right) {
		which_side = false;
	} else {
		which_side = true;
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
	v3f P = min;
	v3f Color = {};
	for (int j = y_start; j < y_end; j++) {
		P.x = Left.x;
		int x_start = round_f32_to_s32(Left.x);
		int x_end = round_f32_to_s32(Right.x);
		for (int i = x_start; i < x_end; i++) {
			Color = barycentric_cood(min, max, mid, P);
			pixel_set(AppBackBuffer, P.xy, Color);
			P.x++;
		}
		Left.x += Left.x_step;
		Right.x += Right.x_step;
		P.y++;
	}


	Left = BottomToTop;
	Right = MiddleToTop;

	// Offset the starting x value of the bottom edge so that it is at the
	// correct x value to render the top half of the triangle
	Left.x += (MiddleToTop.y_start - BottomToTop.y_start) * Left.x_step;

	if (!oriented_right) {
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	y_start = MiddleToTop.y_start;
	y_end = MiddleToTop.y_end;
	P = min;
	Color = {};
	P.y = (f32)y_start;
	for (int j = y_start; j < y_end; j++) {
		P.x = Left.x;
		int x_start = round_f32_to_s32(Left.x);
		int x_end = round_f32_to_s32(Right.x);
		for (int i = x_start; i < x_end; i++) {
			Color = barycentric_cood(min, max, mid, P);
			pixel_set(AppBackBuffer, P.xy, Color);
			P.x++;
		}
		Left.x += Left.x_step;
		Right.x += Right.x_step;
		P.y++;
	}
}

internal void
app_update_and_render(app_back_buffer *AppBackBuffer, app_input *AppInput)
{
	f32 time_delta = AppInput->time_delta;
	f32 direction = 1.0f;
	v3f Color = {1.0f, 1.0f, 1.0f};

	v3f CameraPos = {0.0f, 0.0f, 1.0f};
	v3f CameraDirection = {0.0f, 0.0f, 1.0f};
	v3f CameraUp = {0.0f, 1.0f, 0.0f};


	v3f Camera2Pos = {2.0f, 0.0f, -1.0f};
	v3f Camera2Direction = {-1.0f, 0.0f, 0.0f};
	v3f Camera2Up = {0.0f, 1.0f, 0.0f};

	v3f CameraAbovePos = {0.0f, 5.0f, -1.0f};
	v3f CameraAboveDirection = {0.0f, -1.0f, 0.0f};
	v3f CameraAboveUp = {0.0f, 0.0f, 1.0f};

	// TODO(Justin): changing the camera to produce a different viewing
	// perspective does not produce the expected Result. For eaxmple,.
	// viewing the left down the negative x axis produces an inverted y
	// axis. fix debug this... 
	//
	// Conclusion: for a direction vector positive z values are
	// looking down the z axis and POSITIONS get more and more negative.
	// THIS IS CONFUSING. The direction you are looking has a positive z
	// value but the positions are negative :(

	m4x4 MapToCamera = m4x4_camera_map_create(CameraPos, CameraDirection, CameraUp);
	m4x4 MapToCamera2 = m4x4_camera_map_create(Camera2Pos, Camera2Direction, Camera2Up);
	m4x4 MapToCameraAbove = m4x4_camera_map_create(CameraAbovePos, CameraAboveDirection, CameraAboveUp);

	f32 l = -1.0f;
	f32 r = 1.0f;
	f32 b = -1.0f;
	f32 t = 1.0f;
	f32 n = 1.0f;
	f32 f = 2.0f;

	m4x4 MapToPersp = m4x4_perspective_projection_create(l, r, b, t, n, f);
	m4x4 MapToScreenSpace = m4x4_screen_space_map_create(AppBackBuffer->width, AppBackBuffer->height);
	m4x4 M = MapToScreenSpace * MapToPersp * MapToCamera;

	m4x4 RotateZ = m4x4_identity_create();
	m4x4 Identity = m4x4_identity_create();

	srand(2023);

	triangle Triangles[3];
	Triangles[0].Vertices[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
	Triangles[0].Vertices[1] = {0.5f, 0.0f, 0.0f, 1.0f};
	Triangles[0].Vertices[2] = {0.0f, 0.5f, 0.0f, 1.0f};
	Triangles[0].Color = Color;

	b32 using_camera_one = true;
	b32 using_camera_two = false;
	b32 using_camera_three = false;



	AppInput->is_initiliazed = true;




	// Clear screen to black
	u32 *pixel = (u32 *)AppBackBuffer->memory;
	for (s32 y = 0; y < AppBackBuffer->height; y++) {
		for (s32 x = 0; x < AppBackBuffer->width; x++) {
			*pixel++ = 0;
		}
	}

	//
	// NOTE(Justin): AppInput/ Do something smarter here...
	//

	if (AppInput->Buttons[BUTTON_W].is_down) {
		v3f Shift = {0.0f, 1.0f * time_delta, 0.0f};
		if (using_camera_one) {
			CameraPos += Shift;
		} else if (using_camera_two) {
			Camera2Pos += Shift;
		} else {
			CameraAbovePos += -1.0f * Shift;
		}
	}
	if (AppInput->Buttons[BUTTON_S].is_down) {
		v3f Shift = {0.0f, -1.0f * time_delta, 0.0f};
		if (using_camera_one) {
			CameraPos += Shift;
		} else if (using_camera_two) {
			Camera2Pos += Shift;
		} else {
			CameraAbovePos += -1.0f * Shift;
		}
	}
	if (AppInput->Buttons[BUTTON_A].is_down) {
		v3f Shift = {-1.0f * time_delta, 0.0f, 0.0f};
		if (using_camera_one) {
			CameraPos += Shift;
		} else if (using_camera_two) {
			Camera2Pos += Shift;
		} else {
			CameraAbovePos += Shift;
		}
	}
	if (AppInput->Buttons[BUTTON_D].is_down) {
		v3f Shift = {1.0f * time_delta, 0.0f, 0.0f};
		if (using_camera_one) {
			CameraPos += Shift;
		} else if (using_camera_two) {
			Camera2Pos += Shift;
		} else {
			CameraAbovePos += Shift;
		}
	}
	if (AppInput->Buttons[BUTTON_UP].is_down) {
		v3f Shift = {0.0f, 0.0f, -1.0f * time_delta};
		if (using_camera_one) {
			CameraPos += Shift;
		} else if (using_camera_two) {
			Camera2Pos += Shift;
		} else {
			CameraAbovePos += Shift;
		}
	}
	if (AppInput->Buttons[BUTTON_DOWN].is_down) {
		v3f Shift = {0.0f, 0.0f, 1.0f * time_delta};
		if (using_camera_one) {
			CameraPos += Shift;
		} else if (using_camera_two) {
			Camera2Pos += Shift;
		} else {
			CameraAbovePos += Shift;
		}
	}
	if (AppInput->Buttons[BUTTON_LEFT].is_down) {
		RotateZ = m4x4_rotation_z_create(time_delta);
	} else if (AppInput->Buttons[BUTTON_RIGHT].is_down) {
		RotateZ = m4x4_rotation_z_create(-1.0f * time_delta);
	} else {
		RotateZ = Identity;
	}

	if (AppInput->Buttons[BUTTON_1].is_down) {
		using_camera_one = true;
		using_camera_two = false;
		using_camera_three = false;
	}
	if (AppInput->Buttons[BUTTON_2].is_down) {
		using_camera_one = false;
		using_camera_two = true;
		using_camera_three = false;
	}

	if (AppInput->Buttons[BUTTON_3].is_down) {
		using_camera_one = false;
		using_camera_two = false;
		using_camera_three = true;
	}

	//
	// NOTE(Justin): Update camera transform. Only need to do this
	// whenever a switch happens. Not every frame. TODO(Justin): Fix
	// this so that we do it only whenver a change happens. Do it
	// inside the if-else block app_input logic? Or just count the
	// button presses. That is enough information to determine
	// whether or not there exists a need to switch cameras. If so
	// then can check which camera, if not skip 
	//

	if (using_camera_one) {
		MapToCamera = m4x4_camera_map_create(CameraPos, CameraDirection, CameraUp);
	} else if (using_camera_two) {
		MapToCamera = m4x4_camera_map_create(Camera2Pos, Camera2Direction, Camera2Up);
	} else {
		MapToCamera = m4x4_camera_map_create(CameraAbovePos, CameraAboveDirection, CameraAboveUp);
	}

	//
	// NOTE(Justin): Render axes
	//


#if 0
	step += time_delta * 2 * PI32;
	if (step >= 2 * PI32) {
		step = 0.0f;
	}
#endif

	M = MapToScreenSpace * MapToPersp * MapToCamera;
	v4f Position = {0.0f, 0.0f, -1.0f, 1};

	//
	// NOTE(Justin): Render triangle
	//


	m4x4 RotateY = m4x4_rotation_y_create((time_delta * 2 * PI32 / 4.0f));
	triangle Fragment;
	for (u32 i = 0; i < 3; i++) {
		// NOTE(Justin): We apply any rigid body transformations to
		// the triangle itself. After applying the transformations
		// we obtain a copy of the triangle then apply the viewing
		// transformations and finally the w divide to the
		// copy. If we apply the viewing transformations and
		// w divide to the orignal triangle we can no
		// longer use the data of the triangle unless we undo the
		// w divide and viewing transformation to convert
		// the triangle data back to the proper space. So it is easy
		// to just obtain a copy of the triagnle and apply the
		// viewing and division operations on the triangle and copy
		// this data to the frame buffer. This is the reason for
		// creating the triagnle called Fragment.

		Triangles[0].Vertices[i] = RotateY * Triangles[0].Vertices[i];

		Fragment.Vertices[i] = MapToScreenSpace * MapToPersp * MapToCamera * Triangles[0].Vertices[i];
		Fragment.Vertices[i] = (1.0f / Fragment.Vertices[i].w) * Fragment.Vertices[i];
	}

	triangle_scan_interpolation(AppBackBuffer, &Fragment);

#if 0
	v4f PositionFrag = M * Position;
	PositionFrag = (1.0f / PositionFrag.w) * PositionFrag;

	Color = {1.0f, 1.0f, 1.0f};
	for (u32 i = 0; i < 3; i++) {
		line_draw_dda(&AppBackBuffer, PositionFrag.xy, Fragment.Vertices[i].xy, Color);
	}
#endif

}
