#include <windows.h>

#include "software_renderer_types.h"
#include "software_renderer_math.h"

global_variable b32 GLOBAL_RUNNING;
global_variable win32_back_buffer Win32BackBuffer;
global_variable LARGE_INTEGER tick_frequency;
global_variable input Input;


#if 0
internal void
scan_buffer_fill_between_vertices(scan_buffer *ScanBuffer, v3f VertexMin, v3f VertexMax, b32 which_side)
{
	// 
	// NOTE(Justin): Implementing TOP-LEFT fill convention using the ceiling
	// function.  Is there any easy to implement this? Round and add 1?
	//

	s32 y_start	= (s32)ceil(VertexMin.y);
	s32 y_end	= (s32)ceil(VertexMax.y);
	s32 x_start	= (s32)ceil(VertexMin.x);
	s32 x_end	= (s32)ceil(VertexMax.x);

	f32 y_dist = VertexMax.y - VertexMin.y;
	f32 x_dist = VertexMax.x - VertexMin.x;

	f32 x_step = (f32)x_dist / (f32)y_dist;

	// Distance between real y and first scanline
	f32 yprestep = y_start - VertexMin.y;

	f32 x_current = VertexMin.x + yprestep * x_step;

	s32 *scan_buffer_memory = (s32 *)ScanBuffer->memory;
	for (s32 j = y_start; j < y_end; j++) {
		scan_buffer_memory[2 * j + which_side] = (s32)ceil(x_current);
		x_current += x_step;
	}
}

internal void
scan_buffer_fill_triangle(scan_buffer *ScanBuffer, triangle *Triangle)
{
	v3f tmp = {};
	v3f min = &Triangle->Vertices[0].xyz;
	v3f mid = &Triangle->Vertices[1].xyz;
	v3f max = &Triangle->Vertices[2].xyz;

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

	//
	// NOTE(Justin): To determine whether or not the triangle is right oriented 
	// or left oriented, find the signed area of the triangle using the cross product
	// of two vectors which represent two sides of the triangle. The first vector
	// is the vector FROM MAX TO MIN. The second is FROM MAX TO MID. Note that
	// we are using the sign to determine the orientation of the triangle.
	//

	f32 v0_x = min.x - max.x;
	f32 v0_y = min.y - max.y;

	f32 v1_x = mid.x - max.x;
	f32 v1_y = mid.y - max.y;

	f32 area_double_signed = v0_x * v1_y - v1_x * v0_y;

	b32 oriented_right;
	if (area_double_signed > 0) {
		oriented_right = true;
	} else {
		oriented_right = false;
	}

	//
	// NOTE(Justin): This routine fills the minimmum x values first in the scan
	// buffer. This corresponds to all x values of the side of the triangle that
	// is right oriented and has only one edge. The indexing # to fill the scan buffer is 
	// (2 * j + wihch_side). We want the minimum values to be at offsets 0, 2, 4, ...
	// and so on in the scann buffer. That is why we set which_side to false
	// because then (2 * j + which_side(0))  is even for each j.
	// , initially, whenever the triangle is right oriented. After the min
	// values are entered in the scan buffer the max values for the remaining
	// two sides of the triangle are entered into the scan buffer at offsets 1,B
	// 3, 5, 7, ... which are odd offsets. This is why we set which_side = true
	// after. Then 2 * j + which_side is odd for each value of j. 
	//

	b32 which_side;
	if (oriented_right) {
		which_side = false;
	} else {
		which_side = true;
	}

	scan_buffer_fill_between_vertices(ScanBuffer, min, max, which_side);

	which_side = !which_side;
	scan_buffer_fill_between_vertices(ScanBuffer, mid, max, which_side);
	scan_buffer_fill_between_vertices(ScanBuffer, min, mid, which_side);
}
#endif

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
scan_buffer_draw_shape(win32_back_buffer *Win32BackBuffer, scan_buffer *ScanBuffer, f32 ymin, f32 ymax, v3f Color)
{
	// TODO(Justin): ASSERT 

	int y_min = round_f32_to_s32(ymin);
	int y_max = round_f32_to_s32(ymax);

	if (y_min < 0) {
		y_min = 0;
	}
	if (y_max >= Win32BackBuffer->height) {
		y_max = Win32BackBuffer->height - 1;
	}

	u32 color = color_convert_v3f_to_u32(Color);

	u8 *pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * y_min;
	s32 *scan_buffer_memory = (s32 *)ScanBuffer->memory;
	for (int y = y_min; y < y_max; y++) {
		int x_min = scan_buffer_memory[2 * y];
		int x_max = scan_buffer_memory[2 * y + 1];

		u8 *pixel_start = pixel_row + Win32BackBuffer->bytes_per_pixel * x_min;

		u32 *pixel = (u32 *)pixel_start;
		for (int i = x_min; i < x_max; i++) {
			*pixel++ = color;
		}
		pixel_row += Win32BackBuffer->stride;
	}
}


internal void
pixel_set(win32_back_buffer *Win32BackBuffer, v2f PixelPos, v3f Color)
{
	int x = (int)(PixelPos.x + 0.5f);
	int y = (int)(PixelPos.y + 0.5f);

	if ((x < 0) || (x >= Win32BackBuffer->width)) {
		return;
	}
	if ((y < 0) || (y >= Win32BackBuffer->height)) {
		return;
	}

	u32 color = color_convert_v3f_to_u32(Color);

	u8 *pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * y + Win32BackBuffer->bytes_per_pixel * x;
	u32 *pixel = (u32 *)pixel_row;
	*pixel = color;
}

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

internal void
line_draw_dda(win32_back_buffer *Win32BackBuffer, v2f P1, v2f P2, v3f Color)
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

	pixel_set(Win32BackBuffer, PixelPos, Color);
	for (k = 0; k < steps; k++) {
		PixelPos += Increment;
		pixel_set(Win32BackBuffer, PixelPos, Color);
	}
}

internal void
win32_back_buffer_resize(win32_back_buffer *Win32BackBuffer, int width, int height)
{
	if (Win32BackBuffer->memory) {
		VirtualFree(Win32BackBuffer->memory, 0, MEM_RELEASE);
	}

	Win32BackBuffer->width = width;
	Win32BackBuffer->height = height;
	Win32BackBuffer->bytes_per_pixel = 4;
	Win32BackBuffer->stride = Win32BackBuffer->width * Win32BackBuffer->bytes_per_pixel;

	Win32BackBuffer->Info.bmiHeader.biSize = sizeof(Win32BackBuffer->Info.bmiHeader);
	Win32BackBuffer->Info.bmiHeader.biWidth = Win32BackBuffer->width;
	Win32BackBuffer->Info.bmiHeader.biHeight = Win32BackBuffer->height;
	Win32BackBuffer->Info.bmiHeader.biPlanes = 1;
	Win32BackBuffer->Info.bmiHeader.biBitCount = 32;
	Win32BackBuffer->Info.bmiHeader.biCompression = BI_RGB;


	int Win32BackBuffersize = Win32BackBuffer->width * Win32BackBuffer->height * Win32BackBuffer->bytes_per_pixel;
	Win32BackBuffer->memory = VirtualAlloc(0, Win32BackBuffersize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
}

internal LRESULT CALLBACK
win32_main_window_callback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (Message) {
		case WM_CLOSE:
		case WM_DESTROY:
		{
			GLOBAL_RUNNING = false;
		} break;
		case WM_SIZE:
		{
			RECT Rect;
			GetClientRect(Window, &Rect);

			int client_width = Rect.right - Rect.left;
			int client_height = Rect.bottom - Rect.top;

			win32_back_buffer_resize(&Win32BackBuffer, client_width, client_height);

		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT PaintStruct;
			HDC DeviceContext = BeginPaint(Window, &PaintStruct);

			StretchDIBits(DeviceContext,
					0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
					0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
					Win32BackBuffer.memory,
					&Win32BackBuffer.Info,
					DIB_RGB_COLORS,
					SRCCOPY);

			EndPaint(Window, &PaintStruct);
		} break;
		default:
		{
			Result = DefWindowProcA(Window, Message, wParam, lParam);
		}
	}
	return(Result);
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
circle_draw(win32_back_buffer *Win32BackBuffer, circle Circle, v3f Color)
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
				pixel_set(Win32BackBuffer, test, Color);
			}
		}
	}
}

internal void
rectangle_draw(win32_back_buffer *Win32BackBuffer, rectangle R, v3f Color)
{
	// TODO(Justin): Bounds checking
	u32 x_min = round_f32_to_u32(R.Min.x);
	u32 x_max = round_f32_to_u32(R.Max.x);

	u32 y_min = round_f32_to_u32(R.Min.y);
	u32 y_max = round_f32_to_u32(R.Max.y);


	u32 color = color_convert_v3f_to_u32(Color);

	u8 * pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * y_min + Win32BackBuffer->bytes_per_pixel * x_min;
	for (u32 y = y_min; y < y_max; y++) {
		u32 *pixel = (u32 *)pixel_row;
		for (u32 x = x_min; x < x_max; x++) {
			*pixel++ = color;
		}
		pixel_row += Win32BackBuffer->stride;
	}
}


//TODO(Justin): This is not rendering properly. Moving in and out in the z
//direction warps the axes.

#if 1
internal void
axis_draw(win32_back_buffer *Win32BackBuffer, m4x4 M, v4f Position)
{
	v4f Position2 = {0.0f, 0.0f, Position.z - 1.0f, 1};
	v4f Position3 = {Position.x + 1.0f, 0.0f, Position.z, 1};
	v4f Position4 = {0.0f, Position.y + 1.0f, Position.z, 1};

	Position = M * Position;
	Position = (1.0f / Position.w) * Position;


	Position2 = M * Position2;
	Position2 = (1.0f / Position2.w) * Position2;

	v3f Color = {0.0f, 0.0f, 1.0f};
	line_draw_dda(Win32BackBuffer, Position.xy, Position2.xy, Color);


	Position3 = M * Position3;
	Position3 = (1.0f / Position3.w) * Position3;

	Color = {1.0f, 0.0f, 0.0f};
	line_draw_dda(Win32BackBuffer, Position.xy, Position3.xy, Color);


	Position4 = M * Position4;
	Position4 = (1.0f / Position4.w) * Position4;

	Color = {0.0f, 1.0f, 0.0f};
	line_draw_dda(Win32BackBuffer, Position.xy, Position4.xy, Color);

}
#endif


internal edge
edge_create_from_v3f(v3f VertexMin, v3f VertexMax)
{
	edge Result;
	Result.y_start = (s32)ceil(VertexMin.y);
	Result.y_end = (s32)ceil(VertexMax.y);

	f32 y_dist = VertexMax.y - VertexMin.y;
	f32 x_dist = VertexMax.x - VertexMin.x;

	if (y_dist < 0) {
		Result.x_step = x_dist;
	} else {
		Result.x_step = (f32)x_dist / (f32)y_dist;
	}

	// Distance between real y and first scanline
	f32 yprestep = Result.y_start - VertexMin.y;

	Result.x = VertexMin.x + yprestep * Result.x_step;
	return(Result);
}

internal void
scanline_draw(win32_back_buffer *Win32BackBuffer, edge Left, edge Right, s32 scanline)
{
	s32 x_min = (s32)ceil(Left.x);
	s32 x_max = (s32)ceil(Right.x);

	u8 *pixel_row = (u8 *)Win32BackBuffer->memory + Win32BackBuffer->stride * scanline + Win32BackBuffer->bytes_per_pixel * x_min;
	u32 *pixel = (u32 *)pixel_row;
	for (s32 i = x_min; i < x_max; i++) {
		*pixel++ = 0xFFFFFFFF;
	}
}

#if 0
internal void
triangle_scan(win32_back_buffer *Win32BackBuffer, triangle *Triangle)
{
	v3f tmp = {};
	v3f min = &Triangle->Vertices[0].xyz;
	v3f mid = &Triangle->Vertices[1].xyz;
	v3f max = &Triangle->Vertices[2].xyz;

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
		// Left oriented so swap edges.
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	int y_start = BottomToMiddle.y_start;
	int y_end = BottomToMiddle.y_end;
	for (int j = y_start; j < y_end; j++) {
		scanline_draw(Win32BackBuffer, Left, Right, j);
		Left.x += Left.x_step;
		Right.x += Right.x_step;
	}


	Left = BottomToTop;
	Right = MiddleToTop;

	// Offset the starting x value of the bottom edge so that it is at the
	// correct x value to render the top half of the triangle
	Left.x += (MiddleToTop.y_start - BottomToTop.y_start) * Left.x_step;

	if (!oriented_right) {
		// Left oriented so swap edges.
		edge Temp = Left;
		Left = Right;
		Right = Temp;
	}

	y_start = MiddleToTop.y_start;
	y_end = MiddleToTop.y_end;
	for (int j = y_start; j < y_end; j++) {
		scanline_draw(Win32BackBuffer, Left, Right, j);
		Left.x += Left.x_step;
		Right.x += Right.x_step;
	}
}
#endif

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
triangle_scan_interpolation(win32_back_buffer *Win32BackBuffer, triangle *Triangle)
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
		// Left oriented so swap edges.
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
		int x_start = (int)ceilf(Left.x);
		int x_end = (int)ceilf(Right.x);
		for (int i = x_start; i < x_end; i++) {
			Color = barycentric_cood(min, max, mid, P);
			pixel_set(Win32BackBuffer, P.xy, Color);
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
		// Left oriented so swap edges.
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
		for (int i = Left.x; i < Right.x; i++) {
			Color = barycentric_cood(min, max, mid, P);
			pixel_set(Win32BackBuffer, P.xy, Color);
			P.x++;
		}
		Left.x += Left.x_step;
		Right.x += Right.x_step;
		P.y++;
	}
}
int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSA WindowClass = {};

	WindowClass.style = (CS_HREDRAW | CS_VREDRAW);
	WindowClass.lpfnWndProc = win32_main_window_callback;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = "Main window";

	Win32BackBuffer = {};
	win32_back_buffer_resize(&Win32BackBuffer, 960, 540);

	QueryPerformanceFrequency(&tick_frequency);
	f32 ticks_per_second = (f32)tick_frequency.QuadPart;
	f32 time_for_each_tick = 1.0f / ticks_per_second;

	if (RegisterClassA(&WindowClass)) {
		HWND Window = CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Rasterization Test",
				(WS_OVERLAPPEDWINDOW | WS_VISIBLE),
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				0,
				0,
				hInstance,
				0);

		if (Window) {
			HDC DeviceContext = GetDC(Window);

			scan_buffer ScanBuffer;

			ScanBuffer.memory = VirtualAlloc(0, Win32BackBuffer.height * 2 * sizeof(u32), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
			ScanBuffer.height = Win32BackBuffer.height;


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
			m4x4 MapToScreenSpace = m4x4_screen_space_map_create(Win32BackBuffer.width, Win32BackBuffer.height);
			m4x4 M = MapToScreenSpace * MapToPersp * MapToCamera;

			m4x4 RotateZ = m4x4_identity_create();
			m4x4 Identity = m4x4_identity_create();

			srand(2023);

			triangle Triangles[3];
			Triangles[0].Vertices[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
			Triangles[0].Vertices[1] = {0.5f, 0.0f, 0.0f, 1.0f};
			Triangles[0].Vertices[2] = {0.0f, 0.5f, 0.0f, 1.0f};
			Triangles[0].Color = Color;

			Triangles[1].Vertices[0] = {-0.5f, -0.5f, -1.0f, 1.0f};
			Triangles[1].Vertices[1] = {0.5f, 0.0f, -1.0f, 1.0f};
			Triangles[1].Vertices[2] = {0.0f, 0.5f, -1.0f, 1.0f};
			Triangles[1].Color = Color;

			circle Circle;
			Circle.Center = {0.0f, 0.0f, 0.0f, 1.0f};
			Circle.radius = 10.0f;

			GLOBAL_RUNNING = true;

			b32 using_camera_one = true;
			b32 using_camera_two = false;
			b32 using_camera_three = false;

			b32 last_camera = using_camera_one;

			f32 time_delta = 0.0f;
			f32 step = 0.0f;

			LARGE_INTEGER tick_count_before;
			QueryPerformanceCounter(&tick_count_before);
			while (GLOBAL_RUNNING) {
				MSG Message;
				while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE)) {
					switch (Message.message) {
						case WM_KEYDOWN:
						case WM_KEYUP: 
						{
							u32 vk_code = (u32)Message.wParam;
							b32 was_down = ((Message.lParam & (1 << 30)) != 0);
							b32 is_down = ((Message.lParam & (1 << 31)) == 0);

							if (was_down != is_down) {
								switch (vk_code) {
									case 'W':
									{
										Input.Buttons[BUTTON_W].is_down = is_down;
									} break;
									case 'S':
									{
										Input.Buttons[BUTTON_S].is_down = is_down;
									} break;
									case 'A':
									{
										Input.Buttons[BUTTON_A].is_down = is_down;
									} break;
									case 'D':
									{
										Input.Buttons[BUTTON_D].is_down = is_down;
									} break;
									case VK_UP:
									{
										Input.Buttons[BUTTON_UP].is_down = is_down;
									} break;
									case VK_DOWN:
									{
										Input.Buttons[BUTTON_DOWN].is_down = is_down;
									} break;
									case VK_LEFT:
									{
										Input.Buttons[BUTTON_LEFT].is_down = is_down;
									} break;
									case VK_RIGHT:
									{
										Input.Buttons[BUTTON_RIGHT].is_down = is_down;
									} break;
									case 0x31:
									{
										Input.Buttons[BUTTON_1].is_down = is_down;
									} break;
									case 0x32:
									{
										Input.Buttons[BUTTON_2].is_down = is_down;
									} break;
									case 0x33:
									{
										Input.Buttons[BUTTON_3].is_down = is_down;
									} break;
								}
							}
						} break;
						default:
						{
							TranslateMessage(&Message);
							DispatchMessage(&Message);
						}
					}
				}

				// Clear screen to black
				u32 *pixel = (u32 *)Win32BackBuffer.memory;
				for (s32 y = 0; y < Win32BackBuffer.height; y++) {
					for (s32 x = 0; x < Win32BackBuffer.width; x++) {
						*pixel++ = 0;
					}
				}


#if 0
				// Resize the scan buffer upon a change in the window size
				if (ScanBuffer.height != Win32BackBuffer.height) {
					if (ScanBuffer.memory) {
						VirtualFree(ScanBuffer.memory, 0, MEM_RELEASE);
					}
					ScanBuffer.memory = VirtualAlloc(0, Win32BackBuffer.height * 2 * sizeof(u32), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
					ScanBuffer.height = Win32BackBuffer.height;
					MapToScreenSpace = m4x4_screen_space_map_create(Win32BackBuffer.width, Win32BackBuffer.height);
				}
#endif

				//
				// NOTE(Justin): Input/ Do something smarter here...
				//

				if (Input.Buttons[BUTTON_W].is_down) {
					v3f Shift = {0.0f, 1.0f * time_delta, 0.0f};
					if (using_camera_one) {
						CameraPos += Shift;
					} else if (using_camera_two) {
						Camera2Pos += Shift;
					} else {
						CameraAbovePos += -1.0f * Shift;
					}
				}
				if (Input.Buttons[BUTTON_S].is_down) {
					v3f Shift = {0.0f, -1.0f * time_delta, 0.0f};
					if (using_camera_one) {
						CameraPos += Shift;
					} else if (using_camera_two) {
						Camera2Pos += Shift;
					} else {
						CameraAbovePos += -1.0f * Shift;
					}
				}
				if (Input.Buttons[BUTTON_A].is_down) {
					v3f Shift = {-1.0f * time_delta, 0.0f, 0.0f};
					if (using_camera_one) {
						CameraPos += Shift;
					} else if (using_camera_two) {
						Camera2Pos += Shift;
					} else {
						CameraAbovePos += Shift;
					}
				}
				if (Input.Buttons[BUTTON_D].is_down) {
					v3f Shift = {1.0f * time_delta, 0.0f, 0.0f};
					if (using_camera_one) {
						CameraPos += Shift;
					} else if (using_camera_two) {
						Camera2Pos += Shift;
					} else {
						CameraAbovePos += Shift;
					}
				}
				if (Input.Buttons[BUTTON_UP].is_down) {
					v3f Shift = {0.0f, 0.0f, -1.0f * time_delta};
					if (using_camera_one) {
						CameraPos += Shift;
					} else if (using_camera_two) {
						Camera2Pos += Shift;
					} else {
						CameraAbovePos += Shift;
					}
				}
				if (Input.Buttons[BUTTON_DOWN].is_down) {
					v3f Shift = {0.0f, 0.0f, 1.0f * time_delta};
					if (using_camera_one) {
						CameraPos += Shift;
					} else if (using_camera_two) {
						Camera2Pos += Shift;
					} else {
						CameraAbovePos += Shift;
					}
				}
				if (Input.Buttons[BUTTON_LEFT].is_down) {
					RotateZ = m4x4_rotation_z_create(time_delta);
				} else if (Input.Buttons[BUTTON_RIGHT].is_down) {
					RotateZ = m4x4_rotation_z_create(-1.0f * time_delta);
				} else {
					RotateZ = Identity;
				}

				if (Input.Buttons[BUTTON_1].is_down) {
					using_camera_one = true;
					using_camera_two = false;
					using_camera_three = false;
				}
				if (Input.Buttons[BUTTON_2].is_down) {
					using_camera_one = false;
					using_camera_two = true;
					using_camera_three = false;
				}

				if (Input.Buttons[BUTTON_3].is_down) {
					using_camera_one = false;
					using_camera_two = false;
					using_camera_three = true;
				}

				//
				// NOTE(Justin): Update camera transform. Only need to do this
				// whenever a switch happens. Not every frame. TODO(Justin): Fix
				// this so that we do it only whenver a change happens. Do it
				// inside the if-else block input logic?
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

				//v4f Position = {0.0f, 0.0f, -1.0f, 1};
				
				step += time_delta * 2 * PI32;
				if (step >= 2 * PI32) {
					step = 0.0f;
				}
#if 1
				M = MapToScreenSpace * MapToPersp * MapToCamera;
				v4f Position = {0.0f, 0.0f, -1.0f, 1};
//				axis_draw(&Win32BackBuffer, M, Position);


#endif

				//
				// NOTE(Justin): Render triangle
				//


				RotateZ = m4x4_rotation_z_create(time_delta);// * (PI32 / 4.0f));
				m4x4 RotateY = m4x4_rotation_y_create(time_delta * (PI32 / 4.0f));
				m4x4 RotateX = m4x4_rotation_x_create(time_delta * (PI32 / 4.0f));
				m4x4 Rotate = RotateZ * RotateY * RotateX;
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

#if 0
				Circle.Center = RotateY * Circle.Center;
				circle FragmentCircle;
				FragmentCircle.Center = MapToScreenSpace * MapToPersp * MapToCamera * Circle.Center;
				FragmentCircle.Center = (1.0f / FragmentCircle.Center.w) * FragmentCircle.Center;

				// NOTE(Justin): Scaling of the radius by 1/w simulates a
				// shrinking/growing circle based on if we are moving
				// away/towards the circle.
				FragmentCircle.radius = (1.0f / FragmentCircle.Center.w) * Circle.radius;
				Color = {1.0f, 1.0f, 1.0f};

				circle_draw(&Win32BackBuffer, FragmentCircle, Color);
#endif
				

				triangle_scan_interpolation(&Win32BackBuffer, &Fragment);
#if 0
				triangle_scan(&Win32BackBuffer, &Fragment);

				Color = {1.0f, 1.0f, 1.0f};
				for (u32 i = 0; i < 3; i++) {
					line_draw_dda(&Win32BackBuffer, Position.xy, Fragment.Vertices[i].xy, Color);
				}
#endif
				StretchDIBits(DeviceContext,
						0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
						0, 0, Win32BackBuffer.width, Win32BackBuffer.height,
						Win32BackBuffer.memory,
						&Win32BackBuffer.Info,
						DIB_RGB_COLORS,
						SRCCOPY);

				LARGE_INTEGER tick_count_after;
				QueryPerformanceCounter(&tick_count_after);

				f32 tick_count_elapsed = (f32)(tick_count_after.QuadPart - tick_count_before.QuadPart);
				time_delta = tick_count_elapsed * time_for_each_tick;
				tick_count_before = tick_count_after;
			}
		}
	}
	return(0);
}
