//https://dreamcast.wiki/PowerVR_Introduction

// resolution is 640w x 480h


#include <kos.h>

#include <plx/matrix.h>
#include <plx/prim.h>

#include "vmu_img.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct Color {
	uint8 a;
	uint8 r;
	uint8 g;
	uint8 b;
} color;

KOS_INIT_FLAGS(INIT_DEFAULT);

float position_x = SCREEN_WIDTH/2;
float position_y = SCREEN_HEIGHT/2;

void init(){
	pvr_init_defaults();

	pvr_set_bg_color(1.0,0.5,0.2);
}

void draw_triangle(float x1, float y1,
				   float x2, float y2,
				   float x3, float y3,
				   uint8 a, uint8 r, uint8 g, uint8 b)
				   {
	// POINTS SUBMITTED MUST BE IN CLOCKWISE ORDER
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	vert.flags = PVR_CMD_VERTEX;
	vert.x = x1;
	vert.y = y1;
	vert.z = 5.0f;
	vert.u = 0;
	vert.v = 0;
	vert.argb = PVR_PACK_COLOR(a/255, r/255, g/255, b/255);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x2;
	vert.y = y2;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL; //tells PVR that this will be the final point
	vert.x = x3;
	vert.y = y3;
	pvr_prim(&vert, sizeof(vert));
}

void draw_square(float x1, float x2, float y1, float y2, uint8 a, uint8 r, uint8 g, uint8 b) {
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;

	// x1 = left
	// x2 = right
	// y1 = top
	// y2 = bottom

	if(y1>y2) {
		float swap_y;
		swap_y = y1;
		y1 = y2;
		y2 = swap_y;
	}

	if(x1>x2) {
		float swap_x;
		swap_x = x1;
		x1 = x2;
		x2 = swap_x;
	}

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);

	pvr_prim(&hdr, sizeof(hdr));
	vert.flags = PVR_CMD_VERTEX;
	// bottom left
	vert.x = x1;
	vert.y = y2;
	vert.z = 5.0f;
	vert.u = 0;
	vert.v = 0;
	vert.argb = PVR_PACK_COLOR(a/255, r/255, g/255, b/255);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	// top left
	vert.y = y1;
	pvr_prim(&vert, sizeof(vert));

	// bottom right
	vert.x = x2;
	vert.y = y2;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.y = y1;
	pvr_prim(&vert, sizeof(vert));
}

void draw_square_centered_on(float center_x, float center_y, float width, float height, color argb) {
	float left = center_x - (width/2);
	float right = center_x + (width/2);
	float top = center_y - (height/2);
	float bottom = center_y + (height/2);

	//printf("Left: %f Right: %f Top: %f Bottom: %f\n",left,right,top,bottom);
	//printf("A: %d R: %d G: %d B: %d\n", argb.a, argb.r, argb.g, argb.b);

	draw_square(left, right, top, bottom, argb.a, argb.r, argb.g, argb.b);
}

void draw_vertical_line(float x, float y1, float y2, uint8 a, uint8 r, uint8 g, uint8 b) {
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;
	float swap_y;
	if(y1>y2) {
		swap_y = y1;
		y1 = y2;
		y2 = swap_y;
	}

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);

	pvr_prim(&hdr, sizeof(hdr));
	vert.flags = PVR_CMD_VERTEX;
	vert.x = x;
	vert.y = y2;
	vert.z = 5.0f;
	vert.u = 0;
	vert.v = 0;
	vert.argb = PVR_PACK_COLOR(a/255, r/255, g/255, b/255);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.y = y1;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x+1;
	vert.y = y2;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.y = y1;
	pvr_prim(&vert, sizeof(vert));
}

void draw_horizontal_line(float x1, float x2, float y, uint8 a, uint8 r, uint8 g, uint8 b){
	// Draws a 1-pixel wide horziontal line from x1 to x2 at height y
	// with color determined by rgba

	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;

	// make sure x2 is larger than x1
	float swap_x;
	if (x1 > x2) {
		swap_x = x1;
		x1 = x2;
		x2 = swap_x;
	}
	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);

	pvr_prim(&hdr, sizeof(hdr));

	vert.flags = PVR_CMD_VERTEX;
	// starts at bottom-left corner
	vert.x=x1;
	vert.y=y+1;
	vert.z=5.0f;
	vert.u=0;
	vert.v=0;
	vert.argb=PVR_PACK_COLOR(a/255, r/255, g/255, b/255);
	vert.oargb=0;
	pvr_prim(&vert, sizeof(vert));

	// now top-left corner
	vert.y=y;
	pvr_prim(&vert, sizeof(vert));

	// bottom-right
	vert.x=x2;
	vert.y=y+1;
	pvr_prim(&vert, sizeof(vert));

	vert.flags=PVR_CMD_VERTEX_EOL;

	// top-right
	vert.y = y;
	pvr_prim(&vert, sizeof(vert));
}

int check_buttons(){
	maple_device_t *cont;
    cont_state_t *state;

    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

	if(cont) {
		state=(cont_state_t *)maple_dev_status(cont);

		if(!state){
			return 0;
		}
		if(state->buttons & CONT_DPAD_UP){
			position_y -= 1.0f;
		}
		if(state->buttons & CONT_DPAD_DOWN){
			position_y += 1.0f;
		}
		if(state->buttons & CONT_DPAD_LEFT){
			position_x -= 1.0f;
		}
		if(state->buttons & CONT_DPAD_RIGHT){
			position_x += 1.0f;
		}
	}
	return 0;
}

void draw_frame(){
	check_buttons();

	pvr_wait_ready(); // <-- Prevents those ugly flashes!

	pvr_scene_begin();

	pvr_list_begin(PVR_LIST_OP_POLY);
	//opaque drawing here
	pvr_list_finish();

	pvr_list_begin(PVR_LIST_TR_POLY);
	//translucent drawing here
	
	draw_horizontal_line(100, SCREEN_WIDTH-100, 100, 255, 255, 0, 0); // red - top one
	draw_vertical_line(SCREEN_WIDTH-100, 100, SCREEN_HEIGHT-100, 255, 0, 255, 0); // green - right one
	draw_horizontal_line(100, SCREEN_WIDTH-100, SCREEN_HEIGHT-100, 255, 0, 0, 255); // blue - bottom
	draw_vertical_line(100, SCREEN_HEIGHT-100, 100, 255, 255, 255, 255); // white - left

	// (0,0) is located at top left

	//draw_triangle(0, 0, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 255, 255, 255, 255);

	//draw_square(100, 200, 100, 200, 255, 100, 100, 100);

	color argb;
	argb.a = 255;
	argb.r = 255;
	argb.g = 0;
	argb.b = 255;
	//draw_square_centered_on(150, 150, 30, 30, argb);
	draw_square_centered_on(position_x, position_y, 30, 30, argb);
	

	pvr_list_finish();

	pvr_scene_finish();
}

int main(){

	int exitProgram = 0;

	init();

	printf("Hello world!\n");
	printf("How are you today? :)\n");

	while(!exitProgram){
		maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		cont_state_t *controllerState = (cont_state_t*) maple_dev_status(controller);

		//check controller for START button press
		
		if (controllerState->buttons & CONT_START){
			exitProgram = 1;
		}
		
		draw_frame();
	}

	pvr_shutdown();

}